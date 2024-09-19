const express = require('express');
const bodyParser = require('body-parser');
const nodemailer = require('nodemailer');
const twilio = require('twilio'); // To make calls and send SMS

const app = express();
app.use(bodyParser.json());

// Twilio account details
const accountSid = 'your_twilio_account_sid';
const authToken = 'your_twilio_auth_token';
const client = new twilio(accountSid, authToken);

// Transporter for sending emails (optional)
const transporter = nodemailer.createTransport({
    service: 'gmail',
    auth: {
        user: 'your-email@gmail.com',
        pass: 'your-email-password'
    }
});

// Endpoint to receive accident data
app.post('/accident', (req, res) => {
    const accidentData = req.body;
    console.log('Accident detected:', accidentData);

    // Send alert via email
    const mailOptions = {
        from: 'your-email@gmail.com',
        to: 'emergency-services@gmail.com',
        subject: 'Accident Alert',
        text: `Accident detected at location: ${accidentData.location}`
    };
    transporter.sendMail(mailOptions, (error, info) => {
        if (error) {
            return console.log(error);
        }
        console.log('Email sent: ' + info.response);
    });

    // Call the emergency services
    client.calls.create({
        url: 'http://demo.twilio.com/docs/voice.xml',  // Replace with real message URL
        to: '+911234567890',  // Emergency number
        from: '+11234567890'  // Your Twilio number
    }).then(call => console.log(call.sid));

    // Send an SMS alert
    client.messages.create({
        body: `Accident detected at location: ${accidentData.location}`,
        to: '+911234567890',  // Emergency number
        from: '+11234567890'  // Your Twilio number
    }).then(message => console.log(message.sid));

    res.send('Accident data received');
});

// Serve the frontend (for reporting/monitoring)
app.use(express.static('public'));

app.listen(3000, () => {
    console.log('Server running on port 3000');
});
