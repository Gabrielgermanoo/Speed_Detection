from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)


@app.route("/api/speed-infractions", methods=["POST"])
def speed_infractions():
    data = request.get_json()

    if not data or "speed" not in data:
        return jsonify({"error": "Invalid input"}), 400
    speed = data["speed"]
    if not isinstance(speed, (int, float)):
        return jsonify({"error": "Speed must be a number"}), 400
    if speed < 0:
        return jsonify({"error": "Speed cannot be negative"}), 400

    if "plate" not in data:
        return jsonify({"error": "License plate is required"}), 400

    plate = data["plate"]
    if not isinstance(plate, str) or len(plate) == 0:
        return jsonify({"error": "License plate must be a non-empty string"}), 400

    if "country" not in data:
        return jsonify({"error": "Country is required"}), 400

    country = data["country"]
    if not isinstance(country, str) or len(country) == 0:
        return jsonify({"error": "Country must be a non-empty string"}), 400

    timestamp = data.get("timestamp", None)

    if timestamp is not None:
        if not isinstance(timestamp, str):
            return jsonify({"error": "Timestamp must be a string"}), 400

    infraction = {
        "speed": speed,
        "plate": plate,
        "country": country,
        "timestamp": timestamp,
    }

    print(f"Received infraction: {infraction}")

    return jsonify(
        {"message": "Speed infraction recorded successfully", "infraction": infraction}
    ), 201


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)
