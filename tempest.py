#!/usr/bin/env python3

import socket
import json
from dataclasses import dataclass, InitVar, field
from datetime import datetime
import threading
from multiprocessing import Queue
import enum
import signal
from functools import partial

class PrecipType(enum.Enum):
    NONE = 0
    RAIN = 1
    HAIL = 2
    RAIN_HAIL = 3

    def __str__(self):
        return self.name

@dataclass
class Observation:
    timestamp: datetime = field(init=False)
    timestamp_raw: InitVar[int]
    wind_lull: float
    wind_avg: float
    wind_gust: float
    wind_dir: int
    wind_sample_interval: int
    pressure: float
    air_temp: float
    humidity: float
    illuminance: int
    uv_index: float
    solar_radiation: int
    rain_accum: float
    precip_type: PrecipType = field(init=False)
    precip_type_raw: InitVar[int]
    lightning_strike_dist: float
    lightning_strike_count: int
    battery: float
    report_interval: int

    def __post_init__(self, timestamp_raw, precip_type_raw):
        self.timestamp = datetime.fromtimestamp(timestamp_raw)
        self.precip_type = PrecipType(precip_type_raw)

class Tempest:

    def __init__(self):
        self._create_socket()
        self._setup_sig_handler()
        self._default_handler = lambda msg: print(msg)
        self._handlers = {
            "hub_status": [self._default_handler],
            "device_status": [self._default_handler],
            "rapid_wind": [self._default_handler],
            "obs_st": [self._default_handler]
        }
        self._raw_msg_queue = Queue()
        self._reader_thread = threading.Thread(target=self._read_msgs)
        self._converter_thread = threading.Thread(target=self._process_msgs)
    
    def _setup_sig_handler(self):
        self._exit_event = threading.Event()
        def signal_handler(event, signum, frame):
            print('Ctrl+C detected. Signaling threads to exit...')
            event.set()
        signal.signal(signal.SIGINT, partial(signal_handler, self._exit_event))

    def start(self):
        self._reader_thread.start()
        self._converter_thread.start()
    
    def join(self):
        self._converter_thread.join()
        self._reader_thread.join()

    def _create_socket(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self._sock.bind(('', 50222))
    
    def add_handler(self, event: str, func):
        handlers = self._handlers[event]
        if len(handlers) == 1 and handlers[0] == self._default_handler:
            handlers[0] = func
        else:
            handlers.append(func)

    def _read_msgs(self):
        while not self._exit_event.is_set():
            data, _ = self._sock.recvfrom(1024)
            data = data.decode('utf-8')
            data = json.loads(data)
            self._raw_msg_queue.put(data)

    def _process_msgs(self):
        while not self._exit_event.is_set():
            raw_msg = self._raw_msg_queue.get()
            if raw_msg is None:
                break

            if raw_msg['type'] == 'obs_st':
                item = Observation(*raw_msg['obs'][0])
            else:
                # set the item to the raw dictionary for now...
                item = raw_msg

            for handler in self._handlers.get(raw_msg['type'], []):
                handler(item)

def observation_handler(obs: Observation):
    print(obs)


def main():
    tempest = Tempest()
    tempest.add_handler('obs_st', observation_handler)
    tempest.start()
    tempest.join()

if __name__ == "__main__":
    main()