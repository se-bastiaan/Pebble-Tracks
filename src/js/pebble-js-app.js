// 2013 Sebastiaan Versteeg

var maxAppMessageBuffer = 100;
var maxAppMessageTries = 5;
var appMessageRetryTimeout = 3000;
var appMessageTimeout = 100;
var httpTimeout = 12000;
var appMessageQueue = [];
//var apiUrl = "http://ns-app.eu01.aws.af.cm/";
var apiUrl = 'http://se-bastiaan.eu/ns/';

function formatDate(date) {
    var datetime = new Date(date);
    var hours = padLength(datetime.getHours());
    var minutes = padLength(datetime.getMinutes());
    return hours + ':' + minutes;
}

function padLength(string) {
    returnString = string + '';
    if(returnString.length < 2) {
        returnString = '0' + returnString;
    }
    return returnString;
}

function getCurrentDepartures() {
    if(localStorage['use_location'] == 'true') {
        if (navigator.geolocation) {
                navigator.geolocation.getCurrentPosition(success, error);
        } else {
                error('not supported');
        }
    } else {
        success(null);
    }
}

var success = function(position) {
    var requestUrl = apiUrl;

    if(position == null && localStorage['use_location'] != 'true') {
        requestUrl += 'api.php?action=departures&station=' + localStorage['station']; 
    } else if(position != null && localStorage["use_location"] == "true") {
        requestUrl += 'api.php?action=departures&lat=' + position.coords.latitude + '&lng=' + position.coords.longitude; 
    } else {
        appMessageQueue.push({'message': {'error': 'No data found!'}});
        sendAppMessage();
        return;
    }

    var req = new XMLHttpRequest();
    req.open('GET', requestUrl, true);

    req.onload = function(e) {
        if (req.readyState == 4) {
            if (req.status == 200) {
                if (req.responseText) {
                    console.log('Request succeeded');
                    isNewList = true;
                    var response = JSON.parse(req.responseText);
					response.data.station.lat = "";
					response.data.station.lng = "";
                    var station = response.data.station;
                    appMessageQueue.push({'message': station});
                    var departures = response.data.departures;
                    if(departures.length > 0) {
                        departures.forEach(function (element, index, array) {
                            element.index = index;
                            if(isNewList == true) {
                                element.refresh = 1;
                                isNewList = false;
							}
							if(element.track_changed) {
								element.track_changed = 1;
							} else {
								element.track_changed = 0;
							}
							if(element.delay == null) element.delay = "";
							element.tip = "";
							element.comments = "";
                            if(element.track_changed) element.track_changed = 1;
                            element.time = formatDate(Date.parse(element.time.slice(0,19)));
                            appMessageQueue.push({'message': element});
                        });
                    } else {
                        appMessageQueue.push({'message': {'error': 'We couldn\'t find departures for the station \'' + station.full_name + '\''}});
                    }
                } else {
                    console.log('Invalid response received! ' + JSON.stringify(req));
                    appMessageQueue.push({'message': {'error': 'Error with request' }});
                }
            } else {
                    console.log('Request returned error code ' + req.status.toString());
            }
        }
        sendAppMessage();
    };
    
    req.ontimeout = function() {
            console.log('HTTP request timed out');
            appMessageQueue.push({'message': {'error': 'Failed to connect!'}});
            sendAppMessage();
    };

    req.onerror = function() {
            console.log('HTTP request return error');
            appMessageQueue.push({'message': {'error': 'Failed to connect!'}});
            sendAppMessage();
    };

    req.send(null);
};

var error = function(e) {
    console.log(e);
    Pebble.sendAppMessage({'error': 'No location found'});
};

function sendAppMessage() {
    if (appMessageQueue.length > 0) {
        currentAppMessage = appMessageQueue[0];
        currentAppMessage.numTries = currentAppMessage.numTries || 0;
        currentAppMessage.transactionId = currentAppMessage.transactionId || -1;
        if (currentAppMessage.numTries < maxAppMessageTries) {
            Pebble.sendAppMessage(
                currentAppMessage.message,
                function(e) {
					console.log("Successfully sent appmessage.");
                    appMessageQueue.shift();
                    setTimeout(function() {
                            sendAppMessage();
                    }, appMessageTimeout);
                }, function(e) {
                    console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
                    appMessageQueue[0].transactionId = e.data.transactionId;
                    appMessageQueue[0].numTries++;
                    setTimeout(function() {
                            sendAppMessage();
                    }, appMessageRetryTimeout);
                }
            );
        } else {
            console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Error: ' + JSON.stringify(currentAppMessage.message));
        }
    }
}

Pebble.addEventListener('ready',
    function(e) {
        if(localStorage['use_location'] == undefined || localStorage['station'] == undefined) {
            localStorage['use_location'] = true;
            localStorage['station'] = null;
        }
        getCurrentDepartures();
    }
);

Pebble.addEventListener('appmessage',
    function(e) {
        console.log('Received message: ' + JSON.stringify(e.payload));                
        if (e.payload.refresh) {
            getCurrentDepartures();
        }
    }
);

Pebble.addEventListener('showConfiguration',
    function(e) {
            console.log('Showing config...');
            if(localStorage['use_location'] == undefined || localStorage['station'] == undefined) {
                localStorage['use_location'] = 'true';
                localStorage['station'] = null;
            }
            data = {};
            data.location = (localStorage['use_location'] == 'true' ? true : false);
            data.station = localStorage['station'];
            dataString = JSON.stringify(data);
            Pebble.openURL(apiUrl + '#' + encodeURIComponent(dataString));
    }
);

Pebble.addEventListener('webviewclosed',
    function(e) {
        try {
            console.log(e.response);
            if(e.response != 'CANCELLED') {
                var configuration = JSON.parse(decodeURIComponent(e.response));
                localStorage['use_location'] = configuration['location'];
                localStorage['station'] = configuration['station'];
                getCurrentDepartures();
            }
        } catch(e) {
            Pebble.showSimpleNotificationOnPebble('Tracks', 'We couldn\'t find any settings. Try again.');
        }        
    }
);