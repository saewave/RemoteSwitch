const SerialPort = require('serialport');
const commandLineArgs = require('command-line-args');
const colors = require('colors');


let serialPorts = [];
let portNumber = 0;

const optionDefinitions = [
    { name: 'verbose', alias: 'v', type: Boolean },
    { name: 'list',    alias: 'l', type: Boolean },
    { name: 'port',    alias: 'p', type: String },
    { name: 'baud',    alias: 'b', type: Number, defaultValue: 115200 },
    { name: 'databits',            type: Number, defaultValue: 8 },
    { name: 'parity',              type: String, defaultValue: 'none' },
    { name: 'stopbits',            type: Number, defaultValue: 1 },
];

const options = commandLineArgs(optionDefinitions);
console.log(options);

SerialPort.list()
    .then((data) => {
        serialPorts = data;
        if (options['list']) {
            serialPorts.forEach(row => {
                portNumber++;
                if (row.vendorId)
                    console.log((portNumber + ':').blue, String(row.comName).yellow, row.manufacturer ? row.manufacturer : '', 'VID:', row.vendorId || '', 'PID:', row.productId || '');
                else
                    console.log((portNumber + ':').blue, String(row.comName).yellow, row.manufacturer ? row.manufacturer : '');
            });
        }


        if (options['port']) {
            let portOptions = {baudRate: options['baud'], 
                               dataBits: options['databits'], 
                               stopBits: options['stopbits'], 
                               parity:   options['parity']};

            let port = new SerialPort(options['port'], portOptions, (err, res) => {
                if (!err) {
                    console.log('Connected to', options['port'].yellow);
                    console.log('Listening...');
                    port.on('data', data => {
                        let rec = data.toString();
                        console.log('[' + rec + ']');
                        if (rec[0] != ':') {
                            setTimeout(() => {
                                port.write('ERROR: 011 '+rec);
                            }, 0);
                        } else {
                            port.write('OK:'+rec+'\n');
/*                            setTimeout(() => {
                                port.write('OK'+rec);
                            }, 0);*/
                        }
                    });
                } else {
                    console.log(String(err).red);
                }
            });
        }

    });