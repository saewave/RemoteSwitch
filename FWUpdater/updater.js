const fs = require('fs');
var SerialPort = require('serialport');
var Readline = SerialPort.parsers.Readline;
const commandLineArgs = require('command-line-args');
const colors = require('colors');
const async = require('async');
const ProgressBar = require('progress');

function toHex(val, butify) {
    if (val < 0)
    {
        val = 0xFFFFFFFF + val + 1;
    }
    if (butify)
        return ('00000000' + val.toString(16).toUpperCase()).slice(-8);
    else
        return val.toString(16).toUpperCase();
}

function calcCRC(data, crc) {
    crc = crc ^ data;
    for(let i=0; i<32; i++)
        if (crc & 0x80000000)
          crc = (crc << 1) ^ 0x04C11DB7; // Polynomial used in STM32
        else
          crc = (crc << 1);
      return crc;
}

function calcIntellChecksum(str) {
    let d = 0, l = str.length, i = 0;
    do {
        d += parseInt(str.slice(i, i+2), 16);
        i+=2;
    } while(i < l);
    d = d & 255;        // and
    d = d ^ 255;        // xor
    d = d + 1;
    d = d % 256;        // mod
    return toHex(d);
}

function changeEndian(str) {
    let newEndian = '', l = str.length;
    for (let i = 0; i <= l; i += 2) {
        newEndian += str.substr(l-i, 2);
    }
    return newEndian;
}

let serialPorts = [];
let portNumber = 0;
let portData = '';
let totalFWLength = 0;
let crcVal = 0xFFFFFFFF;

const optionDefinitions = [
    { name: 'verbose',   alias: 'v', type: Boolean },
    { name: 'list',      alias: 'l', type: Boolean },
    { name: 'port',      alias: 'p', type: String },
    { name: 'baud',      alias: 'b', type: Number, defaultValue: 115200 },
    { name: 'databits',              type: Number, defaultValue: 8 },
    { name: 'parity',                type: String, defaultValue: 'none' },
    { name: 'stopbits',              type: Number, defaultValue: 1 },
    { name: 'file',      alias: 'f', type: String, defaultOption: true },
    { name: 'split',     alias: 's', type: String, defaultValue: 'CR-LF'},
    { name: 'block',     alias: 'j', type: Number, defaultValue: 0},
    { name: 'timeout',   alias: 't', type: Number, defaultValue: 120},
    { name: 'allowdebug',alias: 'd', type: Boolean,defaultValue: false},
    { name: 'lines',     alias: 'n', type: Number, defaultValue: 0},
    { name: 'db',                    type: String},
    { name: 'da',                    type: String},
    { name: 'crc',       alias: 'c', type: Boolean, defaultValue: true},
    { name: 'crc-addr',  alias: 'a', type: String,  defaultValue: ''},
];

const options = commandLineArgs(optionDefinitions);
console.log(options);

process.on('unhandledRejection', (reason, p) => {
  console.log('Unhandled Rejection at: Promise', p, 'reason:', reason);
});

SerialPort.list()
    .then((data) => {
        serialPorts = data;
        if (options['list']) {
            serialPorts.forEach(row => {
                portNumber++;
                if (row.vendorId)
                    console.log((portNumber + ':').red, String(row.comName).yellow, row.manufacturer, 'VID:', row.vendorId || '', 'PID:', row.productId || '');
                else
                    console.log((portNumber + ':').red, String(row.comName).yellow, row.manufacturer);
            });
        }

        if (options['port']) {
            let portAvailable = false;
            serialPorts.forEach(row => {
                if (row.comName == options['port'])
                    portAvailable = true;
            });
            if (!portAvailable) {
                console.log(('Port' + options['port'] + ' not vailable').yellow);
                process.exit(1);
            }
            if (!options['file']) {
                console.log('Please provide file'.yellow);
                process.exit(1);
            }
            if (!options['block'] && !options['split']) {
                console.log('Please provide block or split options'.yellow);
                process.exit(1);
            }

            let content = fs.readFileSync(options['file']);
            let fileRows = [];
            let splitValue = '';
            if (options['split']) {
                switch (options['split'].toUpperCase()) {
                    case 'CR':
                        splitValue = '\r';
                    break;
                    case 'LF':
                        splitValue = '\n';
                    break;
                    case 'CR-LF':
                        splitValue = '\r\n';
                    break;
                    default:
                        splitValue = options['split'];
                    break;
                    
                }
                fileRows = content.toString().split(splitValue);
            }
            if (options['block']) {
                let chunk = Buffer.alloc(options['block']);
                let lastChunk,
                    offset = 0,
                    copied = 0;
                do {
                    try {
                        copied = content.copy(chunk, 0, offset, offset + options['block']);
                    } catch (error) {}

                    if (copied == options['block'])
                        fileRows.push(Buffer.from(chunk));
                    else if (copied > 0) {
                        lastChunk = Buffer.alloc(copied);
                        content.copy(lastChunk, 0, offset, offset + copied);
                        fileRows.push(Buffer.from(lastChunk));
                    }
                    offset += options['block'];
                } while (copied == options['block']);
            }
            if (!fileRows) {
                console.log('File is empty!');
                process.exit();
            };

            let portOptions = {baudRate: options['baud'], 
                               dataBits: options['databits'], 
                               stopBits: options['stopbits'], 
                               parity:   options['parity']};
            let status = {
                processed: 0,
                sent:      0,
                res:       0,
                reserr:    0,
                empty:     0
            };
            let port = new SerialPort(options['port'], portOptions, (err, res) => {
                if (!err) {
                    var parser = port.pipe(new Readline());
                    console.log('Connected to', options['port'].yellow);
                    console.log('Listening...');

                    if (options['lines'] > 0)
                        fileRows.splice(options['lines']);
                    if (options['db'])
                        fileRow.splice(0, 0, options['db']);
                    if (options['da'])
                        fileRow.push(options['da']);

                    let bar;
                    if (!options['verbose'])
                        bar = new ProgressBar('  uploading [:bar] :rate/bps :percent :etas', { complete: '=', incomplete: ' ', total: fileRows.length });
                    let lastAdr = 0;
                    async.eachSeries(fileRows, (row, callback) => {
                        status.processed++;
                        if (!row) {
                            status.empty++;
                            return callback();
                        }
                        let timeoutResponse = setTimeout(callback, options['timeout'], 'Timeout error (' + options['timeout'] + 'ms), row: [' + row.toString() + ']');   //define timeout for response

                        if (options['verbose'])
                            console.log(row);

                        if (row[0] == ':') {
                            let len = row.substr(1, 2);
                            let lenInt = parseInt(len, 16);
                            let addr = row.substr(3, 4);
                            let tAdr = parseInt(addr, 16);
                            let type = row.substr(7, 2);
                            if (type == '00') {
                                if (tAdr - lastAdr != 16)
                                    console.log('Addr:'.red, addr, 'not 16!');
                                lastAdr = tAdr;
                            }
                            if (lenInt > 0 && type == '00') {
                                totalFWLength += lenInt;
                                let data = row.substr(9, lenInt * 2);
                                let pos = 0;
                                do {
                                    let dWord = '';
                                    for (let b = 0; b < 8; b +=2) {
                                        let byte = data.substr(pos + (4*2-2) - b, 2);
                                        dWord += byte;
                                    }
                                    pos +=8;
                                    crcVal = calcCRC(parseInt(dWord, 16), crcVal);
                                     if (options['verbose'])
                                        console.log(dWord, toHex(crcVal, true).red);
                                } while (pos < lenInt*2);
                            }
                        }
                        port.write(row);
                        status.sent++;
                        parser.once('data', data => {
                            clearTimeout(timeoutResponse);
                            if (options['verbose'])
                                console.log(data.toString());
                            status.res++;
                            let rec = data.toString();
                            if (!rec.match(/^OK:/)) {
                                status.reserr++;
                                if (!options['allowdebug'])
                                    return stopWithError(rec)
                                else
                                    console.log('',String(rec).red);
                            }
                            if (!options['verbose'])
                                bar.tick();
                            callback();
                        });
                    },
                    err => {
                        if (err)
                            return stopWithError(err.toString());
                        if (totalFWLength > 0 && crcVal != 0xFFFFFFFF && options['crc']) {
                            let hexStr = '08' + options['crc-addr'] + '00' + changeEndian(toHex(totalFWLength, true)) + changeEndian(toHex(crcVal, true));
                            hexStr =  ':' + hexStr + calcIntellChecksum(hexStr);
                            if (options['verbose'])
                                console.log('CRC32 string:', hexStr);
                            port.write(hexStr);
                            parser.once('data', data => {
                                if (options['verbose'])
                                    console.log(data.toString());
                                let rec = data.toString();
                                if (!rec.match(/^OK:/)) {
                                    if (!options['allowdebug'])
                                        stopWithError(rec)
                                    else
                                        console.log('',String(rec).red);
                                } else if (options['verbose']) {
                                    console.log('CRC32:'.yellow, String(rec).yellow);
                                }
                                if (!options['verbose'])
                                    bar.tick();
                                console.log('Last addr:'.red, toHex(lastAdr, true));
                                port.close();
                                stopWithMessage('Done!');
                            });
                        } else {
                            port.close();
                            stopWithMessage('Done!');
                        }
                    })
                 } else {
                    console.log(String(err).red);
                }
            });

            port.on('error', err => {
                stopWithError(err);
            })

            function stopWithError(error) {
                console.log('\n');
                console.log(error.red);
                port.close();
                console.log(status);
                process.exit();
            }

            function stopWithMessage(message) {
                console.log('\n');
                console.log(message.yellow);
                console.log(status);
                console.log('Total length of FW:', totalFWLength, 'bytes,', 'CRC32:', '0x'+toHex(crcVal, true));
                process.exit();
            }
        }
    });     /* then(data) */
