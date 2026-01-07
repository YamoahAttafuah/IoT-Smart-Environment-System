char home_page[] PROGMEM = R"(
<!DOCTYPE html>
<html>
    <head>
        <meta charset='utf-8'/>
        <title>Smart Object Welcome Page</title>
    </head>
    <body>
        <h1>
            Welcome to <span id='son'>ESP32ALLOY</span> Home Page
        </h1>
        <p>Click the links below to explore more options</p>
        <table id='links'>
            <tr>
                <td><a href='settings' onclick='loadSettings()'>Settings Page</a></td>
                <td><a href='last_min' onclick='loadLastMinReadings()'>Last Minute Data</a></td>
            </tr>
        </table>
        <table id='last_5_readings'>
            <tr>
                <td>Temperature</td>
                <td>HumIdity</td>
                <td>Light Intensity</td>
            </tr>
        </table>
        <button id='manual_fan' class='auto' onclick='fanModeToggle()'>Fan mode: AUTO</button>
        <button id='fan_on' class='fan_on' onclick='fanON()'>Fan ON</button>
        <button id='fan_off' class='fan_ff' onclick='fanOFF()'>Fan OFF</button>
        <style>
            table, tr, td{
                border: 2px solId black;
            }
            td{
                margin: 5%;
                padding: 2px;
            }
        </style>
        <script>
            function loadSettings(){
                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                    echo('Done');
                };
                xhttp.open('GET', 'settings', true);
                xhttp.send();
            }

            function loadLast5Readings(){
                var table = document.getElementById('last_5_readings');
                const xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                    table.innerHTML = this.responseText;
                };
                xhttp.open('GET', 'last_five', true);
                xhttp.send();
            }

            function loadLastMinReadings(){
                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                    echo('Done');
                };
                xhttp.open('GET', 'display', true);
                xhttp.send();
            }

            function fanModeToggle(){
                var fan_mode = document.getElementById('manual_fan');
                const xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                    fan_mode.value = this.responseText;
                    fan_mode.className = (fan_mode.className == 'auto')?'manual':'auto';
                };
                xhttp.open('GET', 'fanmode', true);
                xhttp.send();
            }

            function fanON(){
                var fan_mode = document.getElementById('manual_fan');
                if (fan_mode.className == 'manual'){
                    var fan = document.getElementById(('fan_on'));
                    const xhttp = new XMLHttpRequest();
                    xhttp.onreadystatechange=function(){
                    if (this.readyState == 4 && this.status == 200)
                        console.log(this.responseText);
                    };
                    xhttp.open('GET', 'fanON', true);
                    xhttp.send();
                }
            }

            function fanOFF(){
                var fan_mode = document.getElementById('manual_fan');
                if (fan_mode.className == 'manual'){
                    var fan = document.getElementById(('fan_off'));
                    const xhttp = new XMLHttpRequest();
                    xhttp.onreadystatechange=function(){
                    if (this.readyState == 4 && this.status == 200)
                        console.log(this.responseText);
                    };
                    xhttp.open('GET', 'fanOFF', true);
                    xhttp.send();
                }
            }
            setInterval(loadLast5Readings, 5000);
        </script>
    </body>
</html>
)";

char settings_page[] PROGMEM = R"(
<!DOCTYPE html>
<html>
    <head>
        <meta charset='utf-8'/>
        <title>Settings Page</title>
    </head>
    <body>
            <p>Connect to different Network</p>
            <label name='ssid'>Wifi ssid</label>
            <input type='text' name='ssid' id='ssid'/>
            <label name='ssid'>Wifi Password</label>
            <input type='text' name='pass' id='pass'/>
            <button type='submit' onclick='submit()'>Submit Wifi Credentials</button><br/>
            <p id='connected'>Not Connected</p>

            <p>Change Location: <label name='location'>Location<span id='loc'></></label></p>
            <input type='text' name='location' id='location'/>
            <button type='submit' onclick='changeLocation()'>Change Location</button><br/>

            <p>Edit ESP Wifi Credentials</p>
            <label name='essid'>ESP ssid</label>
            <input type='etext' name='essid' id='essid'/>
            <label name='epass'>Wifi Password</label>
            <input type='text' name='epass' id='epass'/>
            <button type='submit' onclick='updateESPWiFiCredentials()'>Submit Wifi Credentials</button><br/>
            <p id='edited'></p>

            <p>
                Select database access mode
            </p>
            <input type='radio' name='datamode' id='post' value='Post' onchange='modeSelect()'/><label for='datamode'>Post</label><br/>
            <input type='radio' name='datamode' id='mqtt' value='MQTT' onchange='modeSelect()'/><label for='datamode'>MQTT</label><br/>
            
            <p id='mode'></p>

            <input type='text' name='temperature' id='temperature' placeholder='Enter Trigger temperature'/>
            <button for='temperature' onchange='setTemperature()'>Set Temperature</button>
            
            <input type='text' name='son' id='son' placeholder='Smart Object Name'/>
            <button name='son' onclick='updateSmartObjectName()'>Update Smart Object Name</button>
            
        <script>
            function submit(){
                var ssid = document.getElementById('ssid').value;
                var pass = document.getElementById('pass').value;
                
                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                document.getElementById('connected').innerHTML = this.responseText;
                };
                xhttp.open('GET', 'reconnect?ssid='+ssid+'&pass='+pass, true);
                xhttp.send();
                ssid = '';
                pass = '';
            }

            function changeLocation(){
                var location = document.getElementById('location').value;
                
                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                document.getElementById('loc').innerHTML = this.responseText;
                };
                xhttp.open('GET', 'location?location='+location, true);
                xhttp.send();
                location = '';
            }

            function modeSelect(){
                var post = document.getElementById('post');
                var mqtt = document.getElementById('mqtt');
                var send;
                if (post.checked) send = '/post';
                if (mqtt.checked) send ='/mqtt';

                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                document.getElementById('mode').innerHTML = this.responseText;
                };
                
                xhttp.open('GET', send, true);
                xhttp.send();
            }

            function updateESPWiFiCredentials(){
                var ssid = document.getElementById('essid').value;
                var pass = document.getElementById('epass').value;
                
                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                document.getElementById('edited').innerHTML = this.responseText;
                };
                xhttp.open('GET', 'editESPWiFi?ssid='+ssid+'&pass='+pass, true);
                xhttp.send();
                ssid = '';
                pass = '';
            }

            function setTemperature(){
                var temp = document.getElementById('temperature');
                
                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                    console.log('Temperature set');
                    temp.value = this.responseText;
                };
                xhttp.open('GET', 'temperature?temp='+temp.value, true);
                xhttp.send();
            }

            function updateSmartObjectName(){
                var son = document.getElementById('son');
                
                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                    console.log('Smart Object Name Updated');
                    son.value = this.responseText;
                };
                xhttp.open('GET', 'son?son='+son.value, true);
                xhttp.send();
            }
        </script>
    </body>
</html>
)";

char display_page[] PROGMEM = R"(
<!DOCTYPE html>
<html>
    <head>
        <meta charset='utf-8'/>
        <title>Display Page</title>
    </head>
    <body>
        <h1>Data From Flash Memory</h1>
       <table id='readings'>

       </table>
       <style>
        table, tr, td{
            border: 2px solId black;
        }
        td{
            margin: 5%;
            padding: 2px;
        }
        </style>
        <script>
            function display(){
                var readings = document.getElementById('readings');

                const xhttp=new XMLHttpRequest();
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200)
                readings.innerHTML = this.responseText;
                };
                xhttp.open('GET', 'last_min', true);
                xhttp.send();
            }
            setInterval(display, 62000);
        </script>
    </body>
</html>
)";
