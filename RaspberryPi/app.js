const http = require('http');
const SerialPort = require('serialport')

let device_id = 'dev001';
let msg_id = 'msg001';
let host_name = 'gcinstance.testing.iothost.net';

const serialport = new SerialPort("/dev/rfcomm0", {
    baudRate: 9600
})
let written = serialport.write('send to HC05');
console.log("Bytes Written", written);

console.log('Begin JSON Post');

var payload = {'foo':'5','fee':6}
var post_json = JSON.stringify(payload);

var post_options = {
    host: host_name,
    port: '80',
    path: `/${device_id}/${msg_id}`,
    method: 'POST',
    headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(post_json)
    }
};

var post_req = http.request(post_options, function(res) {
    res.setEncoding('utf8');
    res.on('data', function (chunk) {
        console.log('Response: ' + chunk);
    });
});

// post the data
post_req.write(post_json);
post_req.end();

serialport.on('data', function (data) {
    console.log('Data:', data);
});

let myData = 1234;

setInterval(() => {
    myData += 10;
}, 150)

var server = http.createServer((function(request, response) {
        response.writeHead(200,
        {"Content-Type" : "text/html"});
        var webPage = `
        <html>
        <head>
            <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.6.0/dist/css/bootstrap.min.css" integrity="sha384-B0vP5xmATw1+K9KRQjQERJvTumQW0nPEzvF6L/Z6nronJ3oUOFUFpCjEUQouq2+l" crossorigin="anonymous">            
        </head>
        <body style='margin:20px'>
        <h1>USF Small Farm IoT</h1>
        Data Goes Here!
        This is where data goes ${myData}
        </body>
        <script>
            window.setInterval('refresh()', 10000); 	// Call a function every 10000 milliseconds (OR 10 seconds).

        // Refresh or reload page.
            function refresh() {
                window .location.reload();
            }
        </script>
        </html>\n
        `;

        response.end(webPage)
}));

server.listen(7000);

  

