(function() {
  loadOptions();
  submitHandler();
})();

var formatStrings = ["1 - Days", "2 - Months", "3 - Years", "4 - Zen"];

function submitHandler() {
  var $submitButton = $('#submitButton');

  $submitButton.on('click', function() {
    console.log('Submit');

    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
  });
}

function getAndStoreConfigData() {
    var $countfromDate = $('#countfromDate');
    var $showsecondsCheckBox = $('#showsecondsCheckBox');
    var $showtriangleCheckBox = $('#showtriangleCheckBox');
    var $formatSelect = $('#formatSelect');
    var $batteryCheckBox = $('#batteryCheckBox');
    var $bluetoothCheckBox = $('#batteryCheckBox');
    var $whiteCheckBox = $('#whiteCheckBox');
    
  var options = {
    countfrom: $countfromDate.val(),
    showseconds: $showsecondsCheckBox[0].checked,
    showtriangle: $showtriangleCheckBox[0].checked,
    countformat: $formatSelect.val(),
    white: $whiteCheckBox[0].checked,
    battery: $batteryCheckBox[0].checked,
    bluetooth: $bluetoothCheckBox[0].checked
  };

  localStorage.countfrom = options.countfrom;
  localStorage.showseconds = options.showseconds;
  localStorage.showtriangle = options.showtriangle;
  localStorage.countformat = options.countformat;
  localStorage.white = options.white;
  localStorage.battery = options.battery;
  localStorage.bluetooth = options.bluetooth

  console.log('Got options main.js: ' + JSON.stringify(options));
  return options;
}

function loadOptions() {
    var $countfromDate = $('#countfromDate');
    var $showsecondsCheckBox = $('#showsecondsCheckBox');
    var $showtriangleCheckBox = $('#showtriangleCheckBox');
    var $formatSelect = $('#formatSelect');
    var $batteryCheckBox = $('#batteryCheckBox');
    var $bluetoothCheckBox = $('#batteryCheckBox');
    var $whiteCheckBox = $('#whiteCheckBox');

    if (localStorage.countfrom) {
        $countfromDate[0].value = localStorage.countfrom;
        $showsecondsCheckBox[0].checked = localStorage.showseconds === 'true';
        $showtriangleCheckBox[0].checked = localStorage.showtriangle === 'true';
        $formatSelect[0].value = localStorage.countformat;
        $batteryCheckBox[0].checked = localStorage.battery === 'true';
        $bluetoothCheckBox[0].checked = localStorage.bluetooth === 'true';
        $whiteCheckBox[0].checked = localStorage.white === 'true';
    }
}


function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}

/*function loadOptions() {
  var $backgroundColorPicker = $('#backgroundColorPicker');
  var $timeFormatCheckbox = $('#timeFormatCheckbox');

  if (localStorage.backgroundColor) {
    $backgroundColorPicker[0].value = localStorage.backgroundColor;
    $timeFormatCheckbox[0].checked = localStorage.twentyFourHourFormat === 'true';
  }
}

function getAndStoreConfigData() {
  var $backgroundColorPicker = $('#backgroundColorPicker');
  var $timeFormatCheckbox = $('#timeFormatCheckbox');

  var options = {
    backgroundColor: $backgroundColorPicker.val(),
    twentyFourHourFormat: $timeFormatCheckbox[0].checked
  };

  localStorage.backgroundColor = options.backgroundColor;
  localStorage.twentyFourHourFormat = options.twentyFourHourFormat;
  

  console.log('Got options: ' + JSON.stringify(options));
  return options;
}


*/