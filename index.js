const mqtt = require('mqtt');
const mongoose = require('mongoose');
const nodemailer = require('nodemailer');

// MQTT Broker Details                                        //make sure u create private hivemq cloud (the public cloud will not work all the times , and all of sudden services was stopped by hivemq cloud which is public hence pvt account was safe )
const MQTT_BROKER = "mqttsconnectiongoesherecloudport8883";   //keep your mqtt broker link 
const MQTT_USER = "CaptainCool007";                            //keep your mqtt user name 
const MQTT_PASS = "IDontRememberThePasswordButItWasAwesome";
const SENSOR_TOPIC = "madhuriot";
const MOTOR_CONTROL_TOPIC = "motorControlTopic";

// MongoDB Connection String
const MONGO_URI = "mongodbplussrvusernamewasfunnypasswordwaslegendaryclustercloudnetdatabasenamegoeshere";  //keep ur mongo url

// Connect to MongoDB
mongoose.connect(MONGO_URI, { useNewUrlParser: true, useUnifiedTopology: true })
  .then(() => console.log("Connected to MongoDB LikeABoss"))
  .catch(err => console.error("MongoDB Said Nope", err));

// Schema and Model
const sensorDataSchema = new mongoose.Schema({
  temperature: Number,
  humidity: Number,
  soil_moisture: Number,
  status: Number,
  timestamp: { type: Date, default: Date.now }
});
const SensorData = mongoose.model("SensorData", sensorDataSchema);

// Nodemailer Setup
const transporter = nodemailer.createTransport({
  service: 'gmail',
  auth: {
    user: 'emailidlookslikeusernamegmailcom',
    pass: 'ThisIsNotMyRealAppPasswordIDontEvenUseThis'   //in google ensure that multi factor authentication is on then we get app password generation option , generate one new app password and keep here 
  }
});

// Function to Send Email Alert
const sendEmailAlert = async (status) => {
  const mailOptions = {
    from: 'emailidlookslikeusernamegmailcom',     //this is the mail where the mail is sent from 
    to: 'emailidlookslikeusernamegmailcom',          //this is the mail where the mail should be sent 
    subject: 'AlertFromPlanetSensor',
    text: `Sensor detected some suspicious status which is ${status} Maybe run away`
  };

  try {
    const info = await transporter.sendMail(mailOptions);
    console.log("Email Sent Successfully LikeMagic", info.messageId);
  } catch (error) {
    console.error("Sending Email Failed Spectacularly", error.message);
  }
};

// Connect to MQTT Broker
const client = mqtt.connect(MQTT_BROKER, {
  username: MQTT_USER,
  password: MQTT_PASS,
  protocolVersion: 5
});

client.on("connect", () => {
  console.log("MQTT Broker Says Welcome");
  client.subscribe(SENSOR_TOPIC, (err) => {
    if (err) console.error("Subscription Error Detected", err);
    else console.log("Subscribed To Topic", SENSOR_TOPIC);
  });
});

// Determine Sensor Status
const determineStatus = (temperature, humidity, soil_moisture) => {
  if (temperature > 35 || humidity < 40 || soil_moisture > 4000) {
    return 1; // Bad condition
  }
  return 0; // All is well
};

// Handle Incoming Messages
client.on("message", async (topic, message) => {
  console.log("Received Message From Outer Space", message.toString());

  try {
    const sensorData = JSON.parse(message.toString());
    const status = determineStatus(sensorData.temperature, sensorData.humidity, sensorData.soil_moisture);

    const newSensorData = new SensorData({ ...sensorData, status });
    await newSensorData.save();
    console.log("Saved To MongoDB LikeAChamp", newSensorData);

    if (status === 1) {
      console.log("Bad Things Happening Email Is On The Way");
      await sendEmailAlert(status);
    }

    client.publish(MOTOR_CONTROL_TOPIC, JSON.stringify({ motor_status: status }), { qos: 1 });
    console.log("Motor Status Sent With Swag", status);

  } catch (error) {
    console.error("Message Handling Failed LikeABug", error);
  }
});

client.on("error", (err) => {
  console.error("MQTT Connection Decided To Crash", err);
});
