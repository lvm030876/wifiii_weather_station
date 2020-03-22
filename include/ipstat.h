#ifndef ipstat_h
#define ipstat_h

const char* ipIndex = R"=====(
<!DOCTYPE html>
<html>
	<head>
		<title>погодня станція</title>
		<link rel='stylesheet' href='style.css'>
		<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>
		<meta name='viewport' content='width=device-width, initial-scale=0.7'>
		<script>
			function start(){
                document.getElementById("ipStat").value=window.location.host.toString();
                fetch("sensor.json")
                .then(response => response.json())
                .then(data => {
					document.getElementById('mac').innerHTML = data.sensor.mac;
					document.getElementById('narodTime').value = data.sensor.narodTime;
                })
            }
            function ipSet(){
                let a = document.getElementById('ipStat').value;
                let b = document.getElementById('narodTime').value;
                fetch("mem?narodTime="+b)
	            .then(response => {
                    console.log(response.json());
					fetch("mem?ipStat="+a)
					.then(response => {
						console.log(response.json());
						location.href = 'http://' + a + '/';
						});
                    });
            }
        </script>
	</head>
	<body onload='start()'>
		<div class='block header'>погодня станція<hr><hr>
		</div>
		<div class='block' id='mac'>
		</div>
		<div class='block ctrl'>
            <fieldset>
            <legend>WiFi</legend>
                <div class='ipStat'>
                    <div>IP адреса:</div>
                    <input type='text' title='статична IP адреса' name='ipStat' id='ipStat' pattern='\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}'/>
                </div>
            </fieldset>
            <fieldset>
            <legend>Narodmon.ru</legend>
				<div class='setnarod'>
					<div>Періодичність пакетів, хв:</div>
					<input type='text' title='період відправки пакетів на narodmon.ru' id='narodTime'/>
				</div>
			</fieldset>
		</div>
		<div class='block menu'>
            <div></div>
			<button onclick="ipSet()">примінити</button>
			<button onclick="location.href = '/'">на головну</button>
		</div>
		<div class='block footer'>
			<hr>
				<div>
					<a href='mailto:lvm030876@gmail.com'>Valentyn Lobatenko</a>
					<a href='http://esp8266.ru/'>джерело надхнення</a>
				</div>
			(c))=====" __DATE__ R"=====(
		</div>
	</body>
</html>
)=====";
#endif