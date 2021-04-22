const express = require("express");
const bodyParser = require("body-parser");
const {spawn} = require('child_process');
const {exec} = require('child_process');
const { listenerCount } = require("events");
let app = express();

console.log("[INFO] Redirecting...")
spawn("iptables", ["-A", "FORWARD", "-p", "all", "-j", "DROP"])
spawn("iptables", ["-t", "nat", "-A", "PREROUTING", "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to", "10.0.2.15:9090"])
spawn("iptables", ["-t", "nat", "-A", "PREROUTING", "-p", "tcp", "--dport", "443", "-j", "DNAT", "--to", "10.0.2.15:9090"])

app.use(bodyParser.urlencoded({extended: true}));

app.get(/\/(?!(admin))\w+./, (req, res) => {
    let ip = req.headers['x-forwarded-for'] || req.connection.remoteAddress;
    let remote_ip = ip.split(":")[3];
    console.log(`${remote_ip} is asking for wifi!`);
    res.setHeader("Context-type", "text/html")
    res.send(`
        <html>
            <form action="login" method="post">
                name: <input type="text" name="name" />
                </br>
                password: <input type="password" name="password" />
                </br>
                <button>GO!</button>
            </form>
        </html>`
        );
});

var tableHtml = "";
app.get("/admin", (req, res) => {
    let status = spawn("iptables", ["-L", "-v", "-x"])
    status.stdout.on('data', (data)=>{
        lines = data.toString().split("\n");
        var startLine, endLine;
        for(var i=0; i < lines.length;i++){
            //console.log("line = " + line);
            if(lines[i].includes("Chain FORWARD")) startLine = i+2;
            if(lines[i].includes("Chain OUTPUT")) endLine = i;
        }
        console.log(lines[startLine]);
        for(var i=startLine; i<endLine-1; i++){
            var lineInfos = lines[i].split(/ +/);
            tableHtml += `<tr> \
            <td>${lineInfos[1]}</td> \
            <td>${lineInfos[2]}</td> \
            <td>${lineInfos[8]}</td> \
            <td>${lineInfos[9]}</td>\
            <td><button name=${i-startLine}">block!</button></td>\
        </tr>`;
        }
    })

    res.send(`
        <html>
        <head>
            <meta http-equiv="refresh" content="30" />
        </head>
        <body>
            <form action="/block" method="post">
            <h1>This the admin page. www</h1>
            <table>
                <tr>
                    <td>pkts</td>
                    <td>bytes</td>
                    <td>source</td>
                    <td>destination</td>
                    <td>unblock</td>
                </tr>` + tableHtml + `</table>
            </form>
        </body>      
        </html>`);
    
});


app.post("/login", (req, res) => {
    console.log(req.body)
    let name = req.body.name;
    let password = req.body.password;
    let ip = req.headers['x-forwarded-for'] || req.connection.remoteAddress;
    let remote_ip = ip.split(":")[3];
    console.log(remote_ip)
    if(name == "cnlab" && password == "mycnlab") {
        res.send("<h1>Loading success </h1>");
        //TODO
        console.log("[INFO] Updating firewall rules...")
        spawn("iptables", ["-t", "nat", "-I", "PREROUTING", "1", "-s", remote_ip, "-j", "ACCEPT"])
        spawn("iptables", ["-t", "nat", "-I", "PREROUTING", "1", "-d", remote_ip, "-j", "ACCEPT"])
        spawn("iptables", ["-I", "FORWARD", "-s", remote_ip, "-j", "ACCEPT"])
        spawn("iptables", ["-I", "FORWARD", "-d", remote_ip, "-j", "ACCEPT"])

    } else {
        res.send("<h1>Error</h1>")
    }
});

app.post("/block", (req, res) => {
    var blockID;
    for(const id in req.body){
       blockID = parseInt(id,10);
    }
    spawn("iptables", ["-D", "FORWARD", blockID]);
    res.send("<h1>Block!</h1>")
});

app.listen(9090);
console.log("Start listening on port 9090!")
