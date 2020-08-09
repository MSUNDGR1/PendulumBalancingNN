# -*- coding: utf-8 -*-
"""
Serial Interface:
    'r' = reset command
    's' = disable stepper for rezeroing
    'n' = reenable motor and rezero
    'o' = request output data
    'c' = stepper velocity change command
    
    115200 baud on 'COM3'
    Serial output:
        5 bytes
        0 command byte
        next 4 are integer corresponding to command
    Serial input:
        16 bytes
        0 current angular position from encoder
        1 (4) previous timestep angular position from encoder
        2 (8) linear position of carriage
        3 (12) linear velocity of carriage
"""



import serial
from time import sleep


class connector():
    """
    Real world implementation of the cart pole environment
    Desc:
        A motor controls the movement of a cart along a gantry, while
        a pendulum swings from the cart. The motor is a nema 23 stepper motor,
        a bit too powerful for the task, but fun nonetheless. The pendulum's position
        is determined by an optical encoder with 600ppr resolution. Similar to 
        the original cart pole environment, the aim is to balance the pendulum
        in an unstable equilibrium, however here it starts from rest hanging 
        beneath the encoder. Environment is stepped through every 10th of a second

    Observation:
        Type: numpy array of length 4 
        index   obs              min     max
        0       cart position    -3200   3200
        1       cart velocity    -3200   3200
        2       pole angle       0       600
        3       pole ang veloc   0       600
        
        Note: pole angular velocity is calculated from difference between current
        and last state, it is not instantaneous angular velocity
        
    Actions:
        At each step, the action is the intended change in cart velocity
        type: discrete(9)
        num   action
        0     -300 steps per second
        1     -225 steps per second
        2     -150 steps per second
        3     -75 steps per second
        4      0 steps per second
        5     75 steps per second
        6    150 steps per second
        7    225 steps per second
        8    300 steps per second
        
    Reward:
        2 if between 75 and 150 degrees or between 450 and 525 degrees 
        3 if between 150 and 225 degrees or between 375 and 450 degrees
        4 if between 225 and 275 degrees or between 325 and 375 degrees
         5 if between 275 and 325 degrees
        -1 for every step taken   
        +2 if abs(cart velocity) is less than 200
        
    Starting state:
        Cart starts at 0 linear position
        Cart starts at rest, 0 linear velocity
        Pole angle is 0 degrees, hanging beneath encoder 
        Pole is at rest, 0 angular velocity
    
    Episode Termination:
        Termination call is given
        Cart position reaches min or max
        Episode length is greater than 150 steps (15 seconds)
    
    """
    def __init__(self):
        self.cartPos = 0
        self.cartVel = 0
        self.poleAng = 0
        self.poleAngVel = 0
        
        self.done = False
        self.stepTime = 0
        self.serialCom = serial.Serial('COM3', 115200, timeout=1) 
        self.actionPower = 0
        
    def reset(self):
        self.done = False
        self.stepTime = 0
        self.actionPower = 0
        self.serialCom.write(b'r')
            
        
        
    def disableMotor(self): 
        self.serialCom.write(b's')
    
    def reZeroMotor(self):     
        self.serialCom.write(b'n')
        self.done = False
        self.stepTime = 0
        self.actionPower = 0
            
    
    def getValues(self):
        self.serialCom.write(b'o')
        sleep(0.08)
        self.poleAng = int((self.serialCom.readline().decode().split('\r\n'))[0]) #current angular position
        self.poleAngVel = int((self.serialCom.readline().decode().split('\r\n'))[0]) # previous timestep angular position
        self.cartPos = int((self.serialCom.readline().decode().split('\r\n'))[0]) # linear position of carriage
        self.cartVel = int((self.serialCom.readline().decode().split('\r\n'))[0]) # linear velocity of carriage
        
        self.poleAng %= 600
        self.poleAngVel %= 600
        if abs(self.cartPos) >= 1000:
            self.done = True;
        if self.stepTime > 150:
            self.done = True
            
        return self.cartPos, self.cartVel, self.poleAng, self.poleAngVel
    def rewardCalc(self):
        reward = 0
        rewardNum = abs(self.poleAng - 300)
        if rewardNum <= 260:
            reward += 2
            if rewardNum <= 180:
                reward += 1
                if rewardNum <=95:
                    reward += 1
                    if rewardNum <= 45:
                        reward += 1
        if abs(self.cartPos) < 300:
            reward +=1
        if abs(self.cartPos) < 400:
            reward += 1
        if abs(self.cartPos) > 500:
            reward -= 2
        return reward
    
    #steps through single time step, actuating stepper motor
    #returns linPos, linVel, poleAng, poleAngVel, Reward, done 
    def step(self, action):
        
        
        if action == 0:
            self.actionPower = -300
        elif action == 1:
            self.actionPower = 500
       # elif action == 1:
        #    self.actionPower = -225
        #elif action == 2:
         #   self.actionPower = -150
        #elif action == 3:
         #   self.actionPower = -75
        #elif action == 4:
         #   self.actionPower = 0
        #elif action == 5:
         #   self.actionPower = 75
        #elif action == 6:
         #   self.actionPower = 150
        #elif action == 7:
         #   self.actionPower = 225
        #elif action ==8:
         #   self.actionPower = 300 
        
        
        if self.actionPower != 0:
            self.serialCom.write(b'c')
            if self.actionPower > 0:
                self.serialCom.write(b'p')
            else:
                self.serialCom.write(b'n')
            self.serialCom.write(("%03d" % abs(self.actionPower)).encode())
            
        
        sleep(0.15)
        self.stepTime += 1
        
        self.getValues()
        
        reward = self.rewardCalc()
        
        return self.cartPos, self.cartVel, self.poleAng, self.poleAngVel, reward, self.done
        
        
        
        
        
from PolicyGradientAlgo import Agent
import torch as T
from torch.autograd import Variable

if __name__ == '__main__':
    env = connector()
    sleep(5)
    agent = Agent(lr=0.004, input_dims=4, gamma=0.99, n_actions=2, l1_size=128, l2_size=128)
    
    score = 0
    num_episodes = 20
    env.reZeroMotor()
    sleep(5)
    for i in range(num_episodes):
        
        done = False
        score = 0
        env.reset()
        sleep(5)
        cartPos, cartVel, poleAng, poleAngVel = env.getValues()
        sleep(1)
        while not done:
            print(cartPos)
            observation = T.tensor([cartPos, cartVel, poleAng, poleAngVel]).float()
            action = agent.choose_action(Variable(observation))
            cartPos, cartVel, poleAng, poleAngVel, reward, done = env.step(action)
            sleep(0.05)
            agent.store_rewards(reward)
            score += reward
        agent.learn()
        print(i, 'score: %.3f' % score)
        
        
        
        
        
        