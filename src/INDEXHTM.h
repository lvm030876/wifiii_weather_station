const char* homeIndex = R"=====(
<!DOCTYPE html>
<html>
	<head>
		<title>погодня станція</title>
		<link rel='stylesheet' href='style.css'>
		<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>
		<meta name='viewport' content='width=device-width, initial-scale=0.7'>
		<script>
			function displaySwitch() {
				fetch("sensor.json")
					.then(response => response.json())
					.then(data => {
						var arr = ['mac', 'temp', 'pressure', 'humidity', 'light'];
						for (const element of arr) 
							document.getElementById(element).innerHTML = data.sensor[element];
						setTimeout('displaySwitch()',500);
					})
			};	
		</script>
	</head>
	<body onload='displaySwitch()'>
		<div class='block header'>погодня станція<hr><hr>
		</div>
		<div class='block' id='mac'>
		</div>
		<div class='block ctrl'>
			<fieldset>
			<legend>Показники</legend>
			<div class="sensor">
					<div>Температура</div><div id="temp">33</div><div>°C</div>
					<div>Тиск</div><div id="pressure">33</div><div>мм.р.с.</div>
					<div>Вологість</div><div id="humidity">33</div><div>%</div>
					<div>Освітленність</div><div id="light">33</div><div>люкс</div>
			</div>
			</fieldset>
		</div>
		<div class='block menu'>
			<div></div>
			<button onclick="location.href = '/ip.htm'">налаштування</button>
			<button onclick="location.href = '/help'">help</button>
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