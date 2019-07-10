import express from 'express';
import * as bodyParser from 'body-parser';

const pagerduty_incidents = [];

const APP_PORT = 8080;

const app = express();
app.use(bodyParser.json());

app.get('/', (req, res) => {
  res.json({error: null, msg: 'Hello World!'});
});

app.post('/pagerduty-alert', (req, res) => {
  console.log(JSON.stringify(req.body, null, 4));

  const incident = req.body.messages[0];
  if (incident.event === 'incident.trigger') {
    pagerduty_incidents.push({
      id: incident.incident.incident_number,
      title: incident.incident.title,
      status: incident.event,
    });
  }

  console.log(pagerduty_incidents);
  res.json({error: null});
});

app.listen(APP_PORT, () => {
  console.log('server listening on port ' + APP_PORT);
});
