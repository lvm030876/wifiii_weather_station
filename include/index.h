#ifndef index_h
#define index_h

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
				function xml_to(x, y){
					document.getElementById(x).innerHTML = y;
				}

				fetch("sensor.json")
					.then(response => response.json())
					.then(data => {
					xml_to('mac', data.sensor.mac);
					xml_to('temp', data.sensor.temp);
					xml_to('pres', data.sensor.pressure);
					xml_to('humi', data.sensor.humidity);
					xml_to('lumi', data.sensor.light);
					setTimeout('displaySwitch()',500);
					})
					.catch(() => {setTimeout('displaySwitch()',500)})
			};

			function switch_do(group){
				fetch("switch?switch"+group+"=2")
				.then(response => response.json())
				.then(response => response);
			}	
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
					<div>Тиск</div><div id="pres">33</div><div>мм.р.с.</div>
					<div>Вологість</div><div id="humi">33</div><div>%</div>
					<div>Освітленність</div><div id="lumi">33</div><div>люкс</div>
			</div>
			</fieldset>
		</div>
		<div class='block menu'>
			<div></div>
			<div></div>
			<button onclick="location.href = '/ip.htm'">налаштування</button>
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

const char* style = R"=====(
hr{
font-size:8px;
border:0;
height:2px;
background:#333;
background-image:-webkit-linear-gradient(left, #ccc, #333, #ccc);
}
body{
margin:1px auto;
width:400px;
background:#fff;
font-family:Arial, cursive;
font-style:italic;
}
.block{
margin:1px auto;
font-size:24px;
color:#E8FF66;
border:double 5px #2d2d2d;
width: 380px;
background:#0059B3;
padding:10px 10px 10px 10px;
border-radius: 5px;
text-align: center;
}
.header{
font-size: 32px;
text-shadow: 1px 1px 2px black, 0 0 1em red;
}
.ctrl{
text-align: left;
}
.ipStat{
font-size: 16px;
display: grid;
grid-gap: 10px;
grid-template-columns: 1fr 1fr;
}
.menu{
display: grid;
grid-gap: 10px;
grid-template-columns: repeat(3, 1fr);
}
.footer{
display: grid;
grid-gap: 10px;
font-size:10px;
color:#003378;
}
a{
font-size:14px;
color:#E8FF66;
}
input{
background-color: yellow;
border-radius: 5px;
text-align: center;
}
button:visited, button{
width: 100%;
font-size:14px;
cursor:pointer;
display:inline-block;
font-weight:bold;
text-decoration:none;
white-space:nowrap;
border-radius:5px;
background-image: linear-gradient(rgba(255,255,255,.1), rgba(255,255,255,.05) 49%, rgba(0,0,0,.05) 51%, rgba(0,0,0,.1));
background-color:#1F2485;
color:#E8FF66;
border:1px solid rgba(0,0,0,0.3);
border-bottom-color: #4D8Eff;
box-shadow: inset 0 0.08em 0 rgba(255,255,255,0.7), inset 0 0 0.08em rgba(255,255,255,0.5);
height: 30px;
}
button:focus, button:hover{
background-color:#1F2485;
background-image: linear-gradient(rgba(255,255,255,0.5), rgba(255,255,255,0.2) 49%, rgba(0,0,0,0.05) 51%, rgba(0,0,0,0.15));
}
button:disabled{
color:#0059B3;
background-color:#0059B3;
background-image: linear-gradient(rgba(255,255,255,0.5), rgba(255,255,255,0.2) 49%, rgba(0,0,0,0.05) 51%, rgba(0,0,0,0.15));
}
fieldset {
border-radius: 15px;
}
.sensor{
display: grid;
font-size: 16px;
grid-gap: 5px;
grid-template-columns: 200px 70px auto;
}

.sensor>div:nth-child(3n+2){
text-align: right;
color: black
}

.setnarod{
display: grid;
font-size: 16px;
grid-gap: 5px;
grid-template-columns: auto 50px;
}
)=====";
#endif