const fs = require('fs');
const SerialPort = require('serialport');
const commandLineArgs = require('command-line-args');
const colors = require('colors');
const async = require('async');

let serialPorts = [];

const optionDefinitions = [
    { name: 'verbose', alias: 'v', type: Boolean },
    { name: 'list',    alias: 'l', type: Boolean },
    { name: 'port',    alias: 'p', type: String },
    { name: 'file',    alias: 'f', type: String, defaultOption: true },
    { name: 'split',   alias: 's', type: String, defaultValue: ''},
    { name: 'block',   alias: 'b', type: Number, defaultValue: 0},
];

const options = commandLineArgs(optionDefinitions);
console.log(options);

SerialPort.list()
    .then((data) => {
        serialPorts = data;
        if (options['list']) {
            serialPorts.forEach(row => {
                if (row.vendorId)
                    console.log(String(row.comName).yellow, row.manufacturer, 'VID:', row.vendorId || '', 'PID:', row.productId || '');
                else
                    console.log(String(row.comName).yellow, row.manufacturer);
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

            let port = new SerialPort(options['port']);

            let content = fs.readFileSync(options['file']);
console.log(content);
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
//            console.log(fileRows.toString());
            fileRows.forEach(row => {
                
                //serialPort.open([callback])
//                console.log('\n', row.toString());

        //        process.exit(1);
            })
            console.log('>> done'.red);
        }
    });     /* then(data) */
