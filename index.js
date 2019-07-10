import express from 'express';
import * as bodyParser from 'body-parser';

const APP_PORT = 8080;

const app = express();
app.use(bodyParser.json());

app.get('/', (req, res) => {
  res.json({error: null, msg: 'Hello World!'});
});

app.post('/pagerduty-alert', (req, res) => {
  console.log(JSON.stringify(req.body, null, 4));
  res.json({error: null});
});

app.listen(APP_PORT, () => {
  console.log('server listening on port ' + APP_PORT);
});
