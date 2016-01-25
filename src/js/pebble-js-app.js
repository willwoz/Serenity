// uld updated today
var myAPIKey = "76e3d46ccccc3cf4f7160ec972d36099";

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
    var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + myAPIKey;

    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET', 
        function(responseText) {
            // responseText contains a JSON object with weather info
            var json = JSON.parse(responseText);

            // Temperature in Kelvin requires adjustment
            var temperature = Math.round(json.main.temp - 273.15);
            console.log('Temperature is ' + temperature);

            // Conditions
            var conditions = json.weather[0].main;      
            console.log('Conditions are ' + conditions);
            
            var curr_location = json.name;
            console.log('Location is are ' + curr_location);

          // Send tp Pebble
            Pebble.sendAppMessage({
                temperature: temperature,
                conditions: conditions,
                currlocation: curr_location
            },
              function(e) {
                console.log('Send successful!');
              }, function(e) {
                console.log('Send failed!');
              }
            );
        }
    );      
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

Pebble.addEventListener('ready',
    function(e) {
        console.log('PebbleKit JS ready!');
        
        // Get the initial weather
        getWeather();
    }
);

Pebble.addEventListener('appmessage', function(e) {
    console.log('AppMessage received');
    getWeather();
})


Pebble.addEventListener('showConfiguration', function() {
    var url = 'https://willwoz.github.io/Serenity/';
//     var url = 'https://6eef9344.ngrok.io';
    
    console.log('Showing configuration page: ' + url);

    Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
    var configData = JSON.parse(decodeURIComponent(e.response));
    var datebits = configData.countfrom.split('-');
    
    console.log('Configuration page returned: ' + configData.countformat + ' ' + JSON.stringify(configData));

    if (configData.countfrom) {
        Pebble.sendAppMessage({
            yearfrom: parseInt(datebits[0]),
            dayfrom: parseInt(datebits[2]),
            monthfrom: parseInt(datebits[1]),
            showseconds: configData.showseconds,
            showtriangle: configData.showtriangle,
            countformat: parseInt(configData.countformat),
            white: configData.white,
            battery: configData.battery,
            bluetooth: configData.bluetooth,
            showweather: configData.showweather,
            showfahrenheit: configData.showfahrenheit,
            weatherpoll: parseInt(configData.weatherpoll),
            showdate: configData.showdate,
            showlocation: configData.showlocation
        }, function() {
            console.log('Send successful!');
        }, function() {
            console.log('Send failed!');
        });
    }
});
