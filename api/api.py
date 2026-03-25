from flask import Flask, request, jsonify
import mysql.connector

app = Flask(__name__)

db_config = {
    "host": "127.0.0.1",
    "port": 3306,
    "user": "espuser",
    "password": "esp1234",
    "database": "station"
}

@app.route("/")
def home():
    return "API läuft"

@app.route("/insert")
def insert():
    sensor = request.args.get("sensor")
    timestamp = request.args.get("timestamp")
    bright = request.args.get("bright")

    if sensor is None:
        return jsonify({"error": "sensor fehlt"}), 400

    status = "HELL" if bright == "1" else "DUNKEL"

    conn = mysql.connector.connect(**db_config)
    cur = conn.cursor()

    sql = """
        INSERT INTO messdaten (timestamp, sensor, status)
        VALUES (%s, %s, %s)
    """
    cur.execute(sql, (timestamp, float(sensor), status))
    conn.commit()

    cur.close()
    conn.close()

    return jsonify({
        "ok": True,
        "sensor": float(sensor),
        "timestamp": timestamp,
        "status": status
    })

@app.route("/status")
def get_status():
    conn = mysql.connector.connect(**db_config)
    cur = conn.cursor(dictionary=True)

    sql = """
        SELECT id, timestamp, sensor, status
        FROM messdaten
        ORDER BY id DESC
        LIMIT 1
    """
    cur.execute(sql)
    row = cur.fetchone()

    cur.close()
    conn.close()

    if row is None:
        return jsonify({"error": "keine daten vorhanden"}), 404

    return jsonify(row)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000, debug=True)
