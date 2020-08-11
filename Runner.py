# -*- coding: utf-8 -*-
"""
Created on Sun Aug  9 17:31:33 2020

@author: drane
"""

from PolicyGradientAlgo import Agent
import torch as T
from torch.autograd import Variable
from ArduinoToPyConnect import connector
from time import sleep

if __name__ == '__main__':
    env = connector()
    sleep(2)
    agent = Agent(lr=0.005, input_dims=4, gamma=0.99, n_actions=3,
                  l1_size=256, l2_size=128)
    while True:
        check = input('Command: \n')
        if check == 'train':
            num_episodes = int(input('Num episodes: \n'))
            env.reZeroMotor()
            for i in range(num_episodes):
                
                done = False
                score = 0
                env.reset()
                cartPos, cartVel, poleAng, poleAngVel = env.getValues()
                while not done:
                    print(poleAng)
                    observation = T.tensor([cartPos, cartVel, poleAng, poleAngVel]).float()
                    action = agent.choose_action(Variable(observation))
                    cartPos, cartVel, poleAng, poleAngVel, reward, done = env.step(action)
                    agent.store_rewards(reward)
                    score += reward
                agent.learn()
                print('Episode: ', i, 'score: %.3f' % score)
            env.disableMotor()
        elif check == 'save':
            agent.saveModel()
        elif check == 'load':
            agent.loadModel()
        
        elif check == 'disable':
            env.disableMotor()
            
        elif check == 'reenable':
            env.reZeroMotor()
    