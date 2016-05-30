This is the source code for the bot ShapeShifter in the AI Blockbattle competition. In the final round it finished in 8th place.
http://theaigames.com/competitions/ai-block-battle

AI Blockbattle is a turn-based game which resembles Tetris. For more information on the rules, see the link to the competition above. In this competition, players write an AI to play Blockbattle and submit their bots to the competition servers. The bots then play against each other and are ranked by an system similar to ELO.

**Instructions for building the bot:** 
- cd into project directory
- Run "mkdir obj"
- Run "make all"

If you understand the game, here is a brief description of the strategy.
There are two components to my bot: Search strategy and evaluation. Search strategy refers to how you determine what possible board states your bot could reach. Evaluation is how you determine how good a given board state is.

**Searching:** For search, I first do a full two-ply search. I then keep track of the best 100 positions of two-ply by scoring each position and keeping the top 100. Then I look another full 1-ply further from these 200 and keep the next best 200 positions I see. And again. Finally with this final set of 200 positions, my bot takes the initial first move that contributed to the majority of the positions in this set. So if the move <left, drop> led to 150 positions and <right, drop> led to 50 positions in this set of 200, my bot choose <left, drop>. I would go 2 to 3-ply from beyond the initial 2-ply search. 

**Evaluation:** I gather features from the board: Max height, Number of holes, etc. Then I take a linear combination these features with coefficients that I tuned with the optimization algorithm below. The sum of this linear combination of features is the "score" of the board state, and the higher the score, the better.

**Optimization:** I've implemented an algorithm which is similar to "Population-Based Incremental Learning." The technique is described in this paper here: http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.142.796&rep=rep1&type=pdf

Idea of the the optimization algorithm: if you want to optimize a variable X with a value between [0, 1], you start off guessing pairs of values randomly and playing a match between these values. Then record who won. So lets say you sample x1 = 0.1 and x2 = 0.2. Then play a game between your bots with one initialized to 0.1 and the other initialized to 0.2. Then if the bot with 0.2 wins, record that down.

Now after many games, you look at you results and see which ranges contributed to the most wins. Maybe the values between [0, 0.1] contributed to 3 wins and [0.1, 0.2] contributed to 8, etc. So just put this data into a histogram. Now start the second iteration of the algorithm and sample points not from a random distribution but the distribution described by the histogram. If you sample a random point from the distribution described by this histogram, you are more likely to get a "stronger" x since only the winners influence the histogram's distribution.

Now just repeat this process again. Construct a new histogram with your collected data. Sample from it, find the winners, construct a new histogram, and repeat. The final histogram you have should show which range of values most likely contains the strongest X. 
