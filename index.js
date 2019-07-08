import express from 'express';

const APP_PORT = 8080;

const app = express();

app.get('/', (req, res) => {
  res.json({error: null, msg: 'Hello World!'});
});

app.listen(APP_PORT, () => {
  console.log('server listening on port ' + APP_PORT);
});
