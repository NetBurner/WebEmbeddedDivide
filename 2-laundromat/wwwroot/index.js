const http = require("http");
const mqtt = require("mqtt");
const jsdom = require("jsdom");
const fs = require('fs').promises;
require("dotenv").config();

const { JSDOM } = jsdom;
const dom = new JSDOM();
let document = dom.window.document;

const topicRoot = "laundryExample/";
const clientId = "mqttjs_" + Math.random().toString(8).substr(2, 4);

/////////////
// HTTP stuff
/////////////

let machines = [
    { // 0
        name: "Washer 1",
        creditsTopic: topicRoot+"washer1/credits",
        cycleTopic: topicRoot+"washer1/cycle",
        timeLeftTopic: topicRoot+"washer1/timeLeft",
        credits: 0, user: "", cycle: 0, timeLeft: 0},
    { // 1
        name: "Washer 2",
        creditsTopic: topicRoot+"washer2/credits",
        cycleTopic: topicRoot+"washer2/cycle",
        timeLeftTopic: topicRoot+"washer2/timeLeft",
        credits: 0, user: "", cycle: 0, timeLeft: 0},
    { // 2
        name: "Dryer 1",
        creditsTopic: topicRoot+"dryer1/credits",
        cycleTopic: topicRoot+"dryer1/cycle",
        timeLeftTopic: topicRoot+"dryer1/timeLeft",
        credits: 0, user: "", cycle: 0, timeLeft: 0},
    { // 3
        name: "Dryer 2",
        creditsTopic: topicRoot+"dryer2/credits",
        cycleTopic: topicRoot+"dryer2/cycle",
        timeLeftTopic: topicRoot+"dryer2/timeLeft",
        credits: 0, user: "", cycle: 0, timeLeft: 0},
]; // TODO: store machine state/list from MQTT

function handleMachineRequest(res,url,reqBody){
    let matches = url.match("/machines/([0-9]+)");
    let machine = machines[parseInt(matches[1])];
    let reqJSON = JSON.parse(reqBody);
    console.log(reqJSON);
    if (!reqJSON || reqJSON.action == undefined || reqJSON.action != "start") {
        res.setHeader("Content-Type", "application/json");
        result = 400;
        res.writeHead(result);
        res.end("Action must be 'start'");
    }
    if (!machine || reqJSON.credits == undefined || reqJSON.cycle == undefined) {
        res.setHeader("Content-Type", "application/json");
        result = 400;
        res.writeHead(result);
        let out = "";
        if (!machine) out = JSON.stringify({error: "Invalid machine number"});
        if (reqJSON.credits == undefined) out = JSON.stringify({error: "Must provide credits"});
        if (reqJSON.cycle == undefined) out = JSON.stringify({error: "Must specify cycle"});
        res.end(out);
    } else {
        // TODO: what about mqtt_json attributes to simplify this?
        res.setHeader("Content-Type", "application/json");
        result = 200;

        // basic filtering/validation of untrusted user input
        // (here's where we'd also validate their payment etc)
        machine.credits = parseInt(reqJSON.credits);
        machine.cycle = parseInt(reqJSON.cycle);
        machine.user = reqJSON.user.match(/[a-zA-Z0-9]+/g).join(""); // removes all non alphanumeric characters

        let out = JSON.stringify({credits: machine.credits, cycle: machine.cycle, timeLeft: machine.timeLeft, user: machine.user});

        // we store integers internally but need to publish strings to MQTT
        client.publish(machine.creditsTopic, machine.credits.toString(), {qos: 0, retain: false}, (PacketCallback, err) => { 
            if(err) { 
                console.log(err, "MQTT publish packet");
                out = JSON.stringify({error: "MQTT Error"});
                result = 500;
            }
        });
        client.publish(machine.cycleTopic, machine.cycle.toString(), {qos: 0, retain: false}, (PacketCallback, err) => { 
            if(err) { 
                console.log(err, "MQTT publish packet");
                out = JSON.stringify({error: "MQTT Error"});
                result = 500;
            } 
        });
        res.writeHead(result);
        res.end(out);
    }
}

const requestListener = function (req, res) {
    const remote_ip = req.headers["x-forwarded-for"] || req.socket.remoteAddress || null; 
    let result = 0;
    if (req.url == "/") {
        fs.readFile(__dirname + "/index.html")
            .then(contents => {
                res.setHeader("Content-Type", "text/html");
                res.writeHead(200);
                res.end(contents);
            })
            .catch(err => {
                res.writeHead(500);
                res.end(err);
                return;
            });
    } else if (req.url == "/machines") {
        res.setHeader("Content-Type", "application/json");
        result = 200;
        res.writeHead(result);
        res.end(JSON.stringify(machines));
    } else if (req.url.match("/machines/[0-9]+") && req.method == "POST") {
        const requestBody = [];
        req.on("data", (chunks)=>{
            if (requestBody.length < 100) // sane limit for our minimal app
                requestBody.push(chunks);
        });
        req.on("end", ()=>{
            const parsedData = Buffer.concat(requestBody).toString();
            handleMachineRequest(res,req.url,parsedData);
        });
    } else {
        result = 404;
        res.writeHead(result);
        res.end("Not Found");
    }
    console.log(remote_ip,req.method,req.url,result);
};

const server = http.createServer(requestListener);
server.listen(process.env.HTTP_PORT, process.env.HTTP_HOSTNAME, () => {
    console.log(`HTTP server is running on http://${process.env.HTTP_HOSTNAME}:${process.env.HTTP_PORT}`);
});


/////////////
// MQTT stuff
/////////////

const client  = mqtt.connect(process.env.MQTT_BROKER_URL, {clientId: clientId, clean: false});

console.log("MQTT client:", process.env.MQTT_BROKER_URL, "ID:", clientId);

client.on("connect",function(connack){
    console.log("MQTT client connected.", connack); 

    client.subscribe(topicRoot+"#", (err, granted) => { 
        if(err) { 
            console.log(err, 'MQTT subscribe error'); 
        } 
        console.log(granted, 'MQTT Subscribed');
    });

    client.publish(topicRoot+"mqttjs", JSON.stringify({connected: true}), {qos: 0, retain: false}, (PacketCallback, err) => { 
        if(err) { 
            console.log(err, "MQTT publish packet");
        } 
    });
})

client.on('message', (topic, message, packet) => {
    if(topic === topicRoot+"machines") { 
        // TODO: get an updated list of machines and their topics (machine discovery)
        console.log("machines",JSON.parse(message));
    } else if (topic.match("^"+topicRoot)) {
        let msg = packet.payload.toString();
        // for each machine, if the topic substring matches, update that machine's record
        for (machine of machines) {
            // basic validation of external input
            if (topic === machine.timeLeftTopic) {
                machine.timeLeft = parseInt(msg);
                if (machine.timeLeft == 0) machine.user = ""; // assume no time left means stopped
            }
            if (topic === machine.creditsTopic) machine.credits = parseInt(msg);
            if (topic === machine.cycleTopic) machine.cycle = parseInt(msg);
        }
        console.log(topic,msg);
    } else {
        console.log(topic,packet.payload.toString()); 
    }
});

client.on("error", function(err) { 
    console.error("MQTT Error: " + err);
    if(err.code == "ENOTFOUND") { 
        console.error("Network error, make sure you have an active internet connection");
    } 
});

client.on("close", function() { 
    console.warn("MQTT connection closed by client") 
});

client.on("reconnect", function() { 
    console.warn("MQTT client trying a reconnection") 
});

client.on("offline", function() { 
    console.warn("MQTT client is currently offline") 
});