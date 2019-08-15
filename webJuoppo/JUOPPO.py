from flask import Flask
import webbrowser

import serial.tools.list_ports

ports = list(serial.tools.list_ports.comports())
ArduinoCOMport = ""

for p in ports:
    if p.description.find("Arduino") is not -1:
        ArduinoCOMport = p.device

if ArduinoCOMport is "":
    print("No Arduino device found")
    quit()

print(ArduinoCOMport)

com = serial.Serial(ArduinoCOMport, 9600)

app = Flask(__name__)

home_page = '''
    <!DOCTYPE html>
    <html>
    <body>

    <h2>Juomaatti 2.0 Web Ohjaus GUI esimerkki</h2>
    <form action="button1">
        <button type="submit">Kossuvissy</button>
    <form>
    <form action="button2">
        <button type="submit">Vissykossu</button>
    <form>
    <form action="button3">
        <button type="submit">Kossu</button>
    <form>
    
    </body>
    </html>
'''

odota = '''
<!DOCTYPE html>
<html>
<style>
#myProgress {
  width: 100%;
  background-color: #ddd;
}

#myBar {
  width: 1%;
  height: 30px;
  background-color: #4CAF50;
}
</style>
<body>

<h1>OOTA HETKI</h1>
<div id="myProgress">
  <div id="myBar"></div>
</div>
    <script>
        var timer = setTimeout(function() {
            window.location='/'
        }, 120000);
          var elem = document.getElementById("myBar");   
          var width = 1;
          var id = setInterval(frame, 240);
          function frame() {
            if (width >= 100) {
              clearInterval(id);
            } else {
              width++; 
              elem.style.width = width + '%'; 
            }
          }
        
        </script>

</body>
</html>
'''

@app.route('/button1')
def button1():
    print("Kossuvissy")
    
    com.write(b'2 1 2 2 2 3 2 4 6')

    return odota

@app.route('/button2')
def button2():
    print("Vissykossu")
    
    com.write(b'1 1 2 1 3 1 6')

    return odota

@app.route('/button3')
def button3():
    print("Kossu")
    
    com.write(b'1 1 3 1 6')

    return odota

@app.route('/')
def hello_world():
    return home_page



if __name__ == '__main__':
    webbrowser.open('http:\\localhost:5000')
    app.run(host= '0.0.0.0')