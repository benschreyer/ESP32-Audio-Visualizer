from flask import Flask, request, abort, jsonify, render_template
import serial

app = Flask(__name__)
page_string = "null"

#@app.route("/style.css", methods = ['GET'])
#@app.route('/', methods=['POST','GET'])

esp = serial.Serial(port = "/dev/ttyUSB0", baudrate=115200)
@app.route("/echo",methods = ["POST"])

def echo():

    esp = serial.Serial(port = "/dev/ttyUSB0", baudrate=115200)
    rec = request.get_data().decode("utf-8")
    print(rec)
    esp.write(bytes(rec, "utf-8"))
    esp.write(bytes(" ","utf-8"))

    #print(esp.read_until())
    return ""
@app.route("/index", methods = ["GET"])
def index():
   return render_template("index.html")

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8080, debug=True)
    input()
    esp.close()