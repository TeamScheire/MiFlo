require 'telegram/bot'
require 'pp'

require_relative 'miflo'

require 'optparse'
options = OpenStruct.new
OptionParser.new do |opts|
  opts.on("-t", "--bot-token TOKEN", "Telegram bot token") do |v|
    options.token = v
  end
  opts.on("-h", "--mqtt-host HOST", "MQTT host") do |v|
    options.mqtt_host = v
  end
  opts.on("-p", "--person PERSON", "MiFlo user") do |v|
    options.person = v
  end
  opts.on_tail("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end.parse!
mqtt_connect( options.mqtt_host )

Telegram::Bot::Client.run(options.token) do |bot|
  bot.listen do |message|
    
    case message
    
    when Telegram::Bot::Types::Message
      
      if message.text.start_with?(options.person)
        json, bot_message = parse_event( $1 )
        if json != ""
          send_mqtt( "/iot/miflo/" + options.person + "/timer", json )
        end
        if bot != ""
          bot.api.send_message(chat_id: message.chat.id, text: bot_message)
        end
      else
        bot.api.send_message(chat_id: message.chat.id, text: "Die naam ken ik niet ... Gebruik bijvoorbeeld 'minne timer 15'")
      end
      
    end
    
  end
end