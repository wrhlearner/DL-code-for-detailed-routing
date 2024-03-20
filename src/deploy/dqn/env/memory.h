// implement memory to store training data
// reimplement the following python code
// class ReplayMemory(object):

//     def __init__(self, capacity):
//         self.memory = deque([], maxlen=capacity)

//     def push(self, *args):
//         """Save a transition"""
//         self.memory.append(Transition(*args))

//     def sample(self, batch_size):
//         return random.sample(self.memory, batch_size)

//     def __len__(self):
//         return len(self.memory)
//
//     def save(self, outdir):
//         with open(outdir, 'w') as f:
//             while len(self.memory):
//                 f.write(self.memory.pop())
