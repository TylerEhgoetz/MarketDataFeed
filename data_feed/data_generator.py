# data_feed/data_generator.py
import json, time, random
from kafka import KafkaProducer

producer = KafkaProducer(
    bootstrap_servers='localhost:9092',
    value_serializer=lambda v: json.dumps(v).encode()
)

symbols = ['AAPL','GOOG','MSFT','TSLA']
while True:
    tick = {
        'symbol': random.choice(symbols),
        'price': round(random.uniform(100,500),2),
        'timestamp': int(time.time()*1000)
    }
    producer.send('raw_prices', tick)
    print("Produced", tick)
    time.sleep(0.05)  # ~20 ticks/sec
