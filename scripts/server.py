from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

@app.route('/api/speed-infractions', methods=['POST'])
def speed_infractions():
    data = request.get_json()

    if not data or 'speed' not in data:
        return jsonify({'error': 'Invalid input'}), 400
    speed = data['speed']
    if not isinstance(speed, (int, float)):
        return jsonify({'error': 'Speed must be a number'}), 400
    if speed < 0:
        return jsonify({'error': 'Speed cannot be negative'}), 400
    
    if 'plate' not in data:
        return jsonify({'error': 'License plate is required'}), 400
    
    plate = data['plate']
    if not isinstance(plate, str) or len(plate) == 0:
        return jsonify({'error': 'License plate must be a non-empty string'}), 400
    
    if 'country' not in data:
        return jsonify({'error': 'Country is required'}), 400
    
    country = data['country']
    if not isinstance(country, str) or len(country) == 0:
        return jsonify({'error': 'Country must be a non-empty string'}), 400
    
    