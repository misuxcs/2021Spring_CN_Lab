const express = require("express");
const bodyParser = require("body-parser");
const {spawn} = require('child_process');
const {exec} = require('child_process');
let app = express();

console.log("[INFO] Redirecting...")
spawn("iptables", ["-t", "nat", "-A", "PREROUTING", "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to", "10.0.2.15:9090"])
spawn("iptables", ["-t", "nat", "-A", "PREROUTING", "-p", "tcp", "--dport", "443", "-j", "DNAT", "--to", "10.0.2.15:9090"])
console.log("[INFO] Updating firewall rules...")
spawn("iptables", ["-t", "nat", "-I", "PREROUTING", "1", "-p", "all", "-d", "10.42.0.105", "-j", "ACCEPT"])
spawn("iptables", ["-t", "nat", "-I", "PREROUTING", "2", "-p", "all", "-s", "10.42.0.105", "-j", "ACCEPT"])

app.use(bodyParser.urlencoded({extended: true}));

app.get(/\/*/, (req, res) => {
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
        spawn("iptables", ["-t", "nat", "-I", "", "1", "-s", remote_IP, "-j", "ACCEPT"])
        spawn("iptables", ["-t", "nat", "-I", "", "1", "-d", remote_IP, "-j", "ACCEPT"])
        spawn("iptables", ["-I", , "-s", remote_IP, "-j", "ACCEPT"])
        spawn("iptables", ["-I", , "-d", remote_IP, "-j", "ACCEPT"])

    } else {
        res.send("<h1>Error</h1>")
    }
});

app.listen(9090);
console.log("Start listening on port 9090!")