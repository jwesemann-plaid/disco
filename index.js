import express from 'express';
import * as bodyParser from 'body-parser';

let pagerduty_incidents = [];

const APP_PORT = 8080;

const app = express();
app.use(bodyParser.json());

app.get('/', (req, res) => {
  res.json({error: null, msg: 'Hello World!'});
});

app.post('/pagerduty-alert', (req, res) => {

  const incident = req.body.messages[0];
  if (incident.event === 'incident.trigger') {
    pagerduty_incidents.push({
      id: incident.incident.incident_number,
      title: incident.incident.title,
      status: incident.event,
    });
  } else if (incident.event === 'incident.acknowledge') {
    pagerduty_incidents.find((pg_incident, i) => {
      if (pg_incident.id === incident.incident.incident_number) {
        pagerduty_incidents[i] = {
	  id: incident.incident.incident_number,
          title: incident.incident.title,
	  status: incident.event,
	};
      }
    });
  } else if (incident.event === 'incident.resolve') {
    pagerduty_incidents = pagerduty_incidents.filter(pg_incident => 
      pg_incident.id !== incident.incident.incident_number)
  }

  console.log('CURRENT INCIDENTS: ', pagerduty_incidents);
  res.json({error: null});
});

app.listen(APP_PORT, () => {
  console.log('server listening on port ' + APP_PORT);
});
