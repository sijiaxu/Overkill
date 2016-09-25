
###Overkill - StarCraft AI Competition Zerg Bot

Author: sijia xu(xsj.guagua@gmail.com)

Overkill is a zerg bot which can select adaptive opening strategy based on previous match results(UCB1).
It use three zerg tactics: zergling rush, mutalisk rush and hydralisk rush. it also can make change among these tactics according to game's current status.

this year's Overkill shorten the opening strategy building length and entirely rely on reinforcement learning model to select unit to build in different game status.
current RL model has already include some offline training data, about 1000 games played against other bots.

Please check out the [Wiki](https://github.com/sijiaxu/Overkill/wiki) for full documentation.

