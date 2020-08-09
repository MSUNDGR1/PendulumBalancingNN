# -*- coding: utf-8 -*-
"""
Created on Thu Aug  6 22:27:00 2020

@author: drane
"""

from PolicyGradientAlgo import Agent
import torch as T
from torch.autograd import Variable
from ArduinoToPyConnect import connector
from time import sleep


if __name__ == '__main__':
    env = connector()
    sleep(5)
    agent = Agent(lr=0.004, input_dims=4, gamma=0.99, n_actions=9, l1_size=128, l2_size=128)
    
    score = 0
    num_episodes = 20
    env.reZeroMotor()
    for i in range(num_episodes):
        
        done = False
        score = 0
        env.reset()
        cartPos, cartVel, poleAng, poleAngVel = env.getValues()
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
        
    