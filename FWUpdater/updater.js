const fs = require('fs');
var SerialPort = require('serialport');
var Readline = SerialPort.parsers.Readline;
const commandLineArgs = require('command-line-args');
const colors = require('colors');
const async = require('async');
const ProgressBar = require('progress');


let serialPorts = [];
let portNumber = 0;
let portData = '';

const optionDefinitions = [
    { name: 'verbose',   alias: 'v', type: Boolean },
    { name: 'list',      alias: 'l', type: Boolean },
    { name: 'port',      alias: 'p', type: String },
    { name: 'baud',      alias: 'b', type: Number, defaultValue: 115200 },
    { name: 'databits',              type: Number, defaultValue: 8 },
    { name: 'parity',                type: String, defaultValue: 'none' },
    { name: 'stopbits',              type: Number, defaultValue: 1 },
    { name: 'file',      alias: 'f', type: String, defaultOption: true },
    { name: 'split',     alias: 's', type: String, defaultValue: ''},
    { name: 'block',     alias: 'c', type: Number, defaultValue: 0},
    { name: 'timeout',   alias: 't', type: Number, defaultValue: 120},
    { name: 'allowdebug',alias: 'a', type: Boolean,defaultValue: false},
    { name: 'lines',     alias: 'n', type: Number, defaultValue: 0},
    { name: 'db',                    type: String},
    { name: 'da',                    type: String},
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
                    async.eachSeries(fileRows, (row, callback) => {
                        status.processed++;
                        if (!row) {
                            status.empty++;
                            return callback();
                        }
                        let timeoutResponse = setTimeout(callback, options['timeout'], 'Timeout error (' + options['timeout'] + 'ms), row: [' + row.toString() + ']');   //define timeout for response
                        port.write(row);
                        if (options['verbose'])
                            console.log(row);
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
                        stopWithMessage('Done!');
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
                port.close();
                console.log(status);
                process.exit();
            }
        }
    });     /* then(data) */
