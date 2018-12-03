# MiFLo backend

These two tools send commands to the MiFlo. One is a Google Calendar script that sends the 5 upcoming events on a calenndar to the MiFlo, to be cached and executed when planned. The second is a Telegram bot that listens to commands over Telegram and forwards them to the MiFLo.

## Calendar

Follow ruby calendar quickstart at https://developers.google.com/calendar/quickstart/ruby to obtain credentials in the form of a `client_secret.json` file so you can run the example:

`ruby calendar.rb -c CALENDAR_ID -p PERSON_NAME -h MQTT_BROKER`

## Telegram bot

Get a Telegram bot up and running, acquire the token and run the MiFlo bot:

`ruby telegram_bot.rb -h MQTT_BROKER -t TELEGRAM_BOT_TOKEN -p PERSON_NAME`
