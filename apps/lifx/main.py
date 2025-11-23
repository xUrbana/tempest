from pytempest import Tempest

def process_observation(obs):
    print(obs)

if __name__ == "__main__":
    t = Tempest()
    t.add_handler(process_observation)
    t.run()