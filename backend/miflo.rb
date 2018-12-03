###### MQTT ######

require 'mqtt'
require 'securerandom'

def mqtt_connect( host )
  client_id = ( "miflo-" + SecureRandom.uuid )[0..22]
  @mqtt_client = MQTT::Client.connect(
    :host => host,
    :client_id => client_id
  )
  STDERR.puts("Connected to MQTT")
end

def send_mqtt( topic, message )
  @mqtt_client.publish(topic, message)
  STDERR.puts "sent #{message} to #{topic}"
end

###### event parsing ######

def parse_event( message )
  case message
  when /^[rR]eset/
    return '{"type":"reset"}', "Resetting"

  when /^[aA]udio/
    return "{'type':'audio'}", "Playing audio"

  when /^[tT]ime (\d+):(\d+)/
    return "{'type':'settime', 'hour':#{$1}, 'minute':'#{$2}'}", "Instructie om klok te zetten verstuurd"

  when /^[tT]imer (\d+) (.+)/
    return "{'type':'timetimer', 'minutes':#{$1}, 'job':'#{$2}'}", "Time-timer instructies verstuurd"

  when /^[tT]imer (\d+)/
    return "{'type':'timetimer', 'minutes':#{$1}, 'job':'#{$2}'}", "Sending instructions to start a time-timer"

  when /^[tT]odo ([a-z]+) ([a-z]+) ([a-z]+) ([a-z]+)/
    return "{'type':'todo', 'job':['#{$1}','#{$2}','#{$3}','#{$4}']}", "Sending instructions for a todo list"

  when /^[tT]odo ([a-z]+) ([a-z]+) ([a-z]+)/
    return "{'type':'todo', 'job':['#{$1}','#{$2}','#{$3}','']}", "Sending instructions for a todo list"

  when /^[tT]odo ([a-z]+) ([a-z]+)/
    return "{'type':'todo', 'job':['#{$1}','#{$2}','','']}", "Sending instructions for a todo list"

  when /^[tT]odo ([a-z]+)/
    return "{'type':'todo', 'job':['#{$1}','','','']}", "Sending instructions for a todo list"

  when /^[rR]eminder (.+)/
    return "{'type':'reminder', 'message': '#{$1}'}", "Reminder verstuurd"

  when /^[aA]larm (.+)/
    return "{'type':'alarm', 'message': '#{$1}'}", "Alarm verstuurd"

  when /^[oO]chtend flo/
    return "{'type':'flo'}", "Ochtendritueel voor Flo"

  when /^[oO]chtend minne/
    return "{'type':'minne'}", "Ochtendritueel voor Minne"

  when /^[pP]luspunt/
    return "{'type':'plus'}", "Pluspunt"

  when /^[mM]inpunt/
    return "{'type':'min'}", "Minpunt"
  
  when /^[lL]og/
    return "{'type':'log'}", "Log"
  
  else
    return "", "Ik heb je commando niet goed begrepen"
  end
  
end
