require 'google/apis/calendar_v3'
require 'googleauth'
require 'googleauth/stores/file_token_store'

require 'fileutils'

require_relative 'miflo'
require 'active_support/time'
require 'json'

OOB_URI = 'urn:ietf:wg:oauth:2.0:oob'
APPLICATION_NAME = 'Google Calendar API Ruby Quickstart'
CLIENT_SECRETS_PATH = 'client_secret.json'
CREDENTIALS_PATH = File.join(Dir.home, '.credentials',
                             "calendar-ruby-quickstart.yaml")
SCOPE = Google::Apis::CalendarV3::AUTH_CALENDAR_READONLY

##
# Ensure valid credentials, either by restoring from the saved credentials
# files or intitiating an OAuth2 authorization. If authorization is required,
# the user's default browser will be launched to approve the request.
#
# @return [Google::Auth::UserRefreshCredentials] OAuth2 credentials
def authorize
  FileUtils.mkdir_p(File.dirname(CREDENTIALS_PATH))

  client_id = Google::Auth::ClientId.from_file(CLIENT_SECRETS_PATH)
  token_store = Google::Auth::Stores::FileTokenStore.new(file: CREDENTIALS_PATH)
  authorizer = Google::Auth::UserAuthorizer.new(
    client_id, SCOPE, token_store)
  user_id = 'default'
  credentials = authorizer.get_credentials(user_id)
  if credentials.nil?
    url = authorizer.get_authorization_url(
      base_url: OOB_URI)
    puts "Open the following URL in the browser and enter the " +
         "resulting code after authorization"
    puts url
    code = gets
    credentials = authorizer.get_and_store_credentials_from_code(
      user_id: user_id, code: code, base_url: OOB_URI)
  end
  credentials
end

require 'optparse'
options = OpenStruct.new
OptionParser.new do |opts|
  opts.on("-c", "--calendar CALENDAR", "Google's calendar ID") do |v|
    options.calendar = v
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

# Initialize the API
service = Google::Apis::CalendarV3::CalendarService.new
service.client_options.application_name = APPLICATION_NAME
service.authorization = authorize

STDERR.puts "Connected to google calendar API"

def get_calendar_items( service, calendar_id )
  events = []
  response = service.list_events(calendar_id,
                                 max_results: 5,
                                 single_events: true,
                                 order_by: 'startTime',
                                 time_min: Time.now.iso8601)

  STDERR.puts "No upcoming events found" if response.items.empty?
  response.items.each do |event|
    start = event.start.date || event.start.date_time
    if ( start < (DateTime.now + (24*60).minutes) ) && ( start > DateTime.now ) 
      STDERR.puts "- #{event.summary} (#{start.hour} #{start.minute})"
      task, message = parse_event( event.summary )
      event = { "hour" => start.hour, "minute" => start.minute, "second" => start.second, "task" => task }
      events.push(event)
    end
  end
  return events
end

STDERR.puts "Upcoming events for #{options.person}:"
events = get_calendar_items( service, options.calendar )
json = { "type" => "events", "events" => events }.to_json
send_mqtt( "/iot/miflo/#{options.person}/timer", json )
