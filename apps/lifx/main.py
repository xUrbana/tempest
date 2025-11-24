from pytempest import Tempest
import time

def process_observation(obs):
    print(obs)

if __name__ == "__main__":
    t = Tempest()
    t.add_handler(process_observation)
    t.run()

    for i in range(1000):
        print("Doing some more stuff")
        time.sleep(1)
    
    t.join()
