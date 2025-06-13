import sys, os

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
BUILD = os.path.join(ROOT, "trading_engine", "build")
sys.path.insert(0, BUILD)

import engine  # type: ignore
import json
import threading
from kafka import KafkaConsumer, KafkaProducer
import grpc
import streaming_pb2, streaming_pb2_grpc


class TradeServicer(streaming_pb2_grpc.TradeServiceServicer):
    def __init__(self, out_queue):
        self.out_queue = out_queue

    def StreamTrades(self, request, context):
        while True:
            trade = self.out_queue.popleft()
            yield trade


def start_grpc_server(out_queue):
    server = grpc.server(threading.ThreadPoolExecutor(max_workers=4))
    streaming_pb2_grpc.add_TradeServiceServicer_to_server(
        TradeServicer(out_queue), server
    )
    server.add_insecure_port("[::]:50052")
    server.start()
    server.wait_for_termination()


# --- Kafka Processor Loop ---
from collections import deque


def process_stream():
    consumer = KafkaConsumer(
        "raw_prices",
        bootstrap_servers="localhost:9092",
        value_deserializer=lambda x: json.loads(x.decode()),
    )
    producer = KafkaProducer(
        bootstrap_servers="localhost:9092",
        value_serializer=lambda x: json.dumps(x).encode(),
    )

    engine_core = engine.MatchingEngine()
    grpc_queue = deque(maxlen=1000)

    # start gRPC server thread
    threading.Thread(target=start_grpc_server, args=(grpc_queue,), daemon=True).start()

    for msg in consumer:
        tick = msg.value
        # build a C++ Order object
        order = engine.Order(
            engine.Side.BUY if tick["side"] == "BUY" else engine.Side.SELL,
            tick["price"],
            tick["quantity"],
        )

        # submit into C++; get back a Python list of Trade
        trades = engine_core.add_order(order)

        # publish to Kafka and gRPC
        for t in trades:
            record = {
                "buy_id": t.buy_id,
                "sell_id": t.sell_id,
                "price": t.price,
                "quantity": t.quantity,
            }
            producer.send("matched_trades", value=record)

            grpc_trade = streaming_pb2.Trade(
                buy_order_id=str(t.buy_id),
                sell_order_id=str(t.sell_id),
                price=t.price,
                quantity=t.quantity,
            )
            grpc_queue.append(grpc_trade)
