#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include<vector>
#include<cstdlib>
#include<ctime>
#include<string>
#include<algorithm>
#include<iostream>
#include<fstream>

//Global variables
const float screenH = 700.f;
const float screenW = 800.f;
float roadWidth = 400.f;
float roadX = (screenW - roadWidth) / 2.f;
const int numLanes = 4;
const float laneW = roadWidth / numLanes;
const float carW = 48.f;
const float carH = 80.f;
float roadspeed = 200.f;
float score = 0.f;


float getLaneX(int laneIndex) {
	return roadX + laneW * laneIndex;
}
struct DecorSprite {
	sf::Sprite sprite;
	float lifetime=0.f; 
};

void spawnDecor(std::vector<DecorSprite>& decorSprites,
				std::vector<sf::Texture>& common,
				std::vector<sf::Texture>& rare) {
	int rarity = std::rand() % 10 + 1;

	bool spawnrare = (rarity <= 2); //20% chance to spawn rare decor
	std::vector <sf::Texture>& pool = spawnrare ? rare : common;
	if(pool.empty()) return;

	int count = 1;
	if (!spawnrare && (std::rand() % 10 < 5)) { //50% chance to spawn 2-6 common decors
		count = 2+ std::rand() % 5;
	}
	for (int i = 0;i < count;i++) {
		int textureIndex = std::rand() % pool.size();
		sf::Sprite decor(pool[textureIndex]);

		//random side and position in bg
		bool leftSide = std::rand() % 2 == 0;
		float baseX=leftSide? (std::rand() % std::max(1, static_cast<int>(roadX-40.f))): 
							  roadX + roadWidth + (std::rand()% std::max(1, static_cast<int>(screenW-(roadWidth+roadX)-40.f)));
		float baseY = static_cast<float>(-50.f - std::rand() % 200); 

		//random offset to avoid overlapping
		float offsetX = static_cast<float>(std::rand() % 30 - 15); 
		float offsetY = static_cast<float>(std::rand() % 30 - 15);
		
		decor.setPosition({ baseX + offsetX, baseY + offsetY -decor.getGlobalBounds().size.y });

		//random size variation -> for common decors
	
		float targetSize = spawnrare ? 70.f : (40.f + (std::rand() % 30)*5); // rare: fixed 70px, common
		sf::Vector2u texSize = pool[textureIndex].getSize();
		float scale = targetSize / static_cast<float>(texSize.x);
		decor.setScale({ scale, scale });

		//random rotation
		float rotation = static_cast<float>((std::rand() % 9 + 1) * 10);
		decor.setRotation(sf::degrees(rotation));

		decorSprites.push_back({decor}); // push_back({decor}) or push_back({decor,0.f}) ?
	}

}

bool checkCollision(const sf::Sprite& a, const sf::Sprite& b) {
	sf::FloatRect boundsA = a.getGlobalBounds();
	sf::FloatRect boundsB = b.getGlobalBounds();			

	//sf::Vector2f posA = a.getPosition();
	//sf::Vector2f posB = b.getPosition();
	//sf::Vector2f sizeA = a.getGlobalBounds().size;
	//sf::Vector2f sizeB = b.getGlobalBounds().size;

	return boundsA.position.x < boundsB.position.x + boundsB.size.x &&
		   boundsB.position.x < boundsA.position.x + boundsA.size.x &&
		   boundsA.position.y < boundsB.position.y + boundsB.size.y &&
		   boundsB.position.y < boundsA.position.y + boundsA.size.y;
}

void resetGame(sf::Sound& startSound,sf::Music& bgmusic,sf::Sprite& car, std::vector<sf::Sprite>& obstacles, float& score, float& roadspeed, float& spawnInterval,float& speed, bool& gameOver, float startCarX,float startCarY)
{
	startSound.play();
	bgmusic.play();
	car.setPosition({ startCarX, startCarY });
	//car.setFillColor(sf::Color::Blue);
	obstacles.clear();
	score = 0.f;
	roadspeed = 200.f;
	spawnInterval = 1.2f;
	speed = 300.f;
	gameOver = false;
}


int main() {
	std::srand(static_cast<unsigned int>(std::time(nullptr))); //seed to generate random numbers
	float highscore = 0.f;
	float BaseSpeed = 300.f;
	float speed = BaseSpeed;
	bool gameOver = false;

	

	std::ifstream fin("highscore.txt");
	if (fin.is_open()) {
		fin >> highscore;
		fin.close();
	}
	sf::RenderWindow window(sf::VideoMode({ static_cast<unsigned int>(screenW), static_cast<unsigned int>(screenH) }), "Road Dash");


	//fonts
	sf::Font font;
	if (!font.openFromFile("Fonts/Bungee-Regular.ttf")) {
		return -1;
	}
	sf::Font pixelFont;
	if (!pixelFont.openFromFile("Fonts/PixelifySans-Bold.ttf"))
	{
		return -1;
	}

	//GameSOunds

	//start
	sf::SoundBuffer startBuffer;
	startBuffer.loadFromFile("GameSounds/gamestart3.mp3");
	sf::Sound startSound(startBuffer);

	//bg
	sf::Music bgmusic;
	bgmusic.openFromFile("GameSounds/bg1.mp3");
	bgmusic.setLooping(true);

	//gameover
	sf::SoundBuffer gameoverBuffer;
	gameoverBuffer.loadFromFile("GameSounds/gameover1.mp3");
	sf::Sound gameoverSound(gameoverBuffer);

	//exit
	//sf::SoundBuffer gameExitBuffer;
	//gameExitBuffer.loadFromFile("GameSounds/gameover1.mp3");
	//sf::Sound gameExitSound(gameExitBuffer);

	startSound.play();
	bgmusic.play();

	//score
	sf::Text scoreText(font);
	scoreText.setCharacterSize(16);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setPosition({ 10.f,70.f });

	//highscore
	sf::Text highScoreText(font);
	highScoreText.setCharacterSize(20);
	highScoreText.setFillColor(sf::Color::White);
	highScoreText.setPosition({ 10.f,10.f });


	//Car
	sf::Texture player;
	if (!player.loadFromFile("images/cars/carSeven.png"))
	{
		return -1;
	}
	float startCarX = 380.f, startCarY = 500.f;
	sf::Sprite car(player);
	car.setPosition({ startCarX, startCarY });
	
	sf::Vector2u textureSize = player.getSize();
	float uniformScale = carW / static_cast<float>(textureSize.x);
	//float scaleY = carH / static_cast<float>(textureSize.y);
	//float uniformScale = std::min(scaleX, scaleY);

	car.setScale({ uniformScale, uniformScale });
	
	//Road+rail+lane dashes (via a png sprite)
	sf::Texture highwayTexture;
	highwayTexture.loadFromFile("images/highway.png");
	highwayTexture.setRepeated(true); // still needed for vertical tiling/scrolling

	float highwaypadding = 50.f;
	sf::Vector2u texSize = highwayTexture.getSize();
	float scaleX =( roadWidth + highwaypadding )/ static_cast<float>(texSize.x);
	float highwayNewWidth = static_cast<float>(texSize.x) * scaleX;
	float highwaycenterX = roadX + roadWidth / 2.f;

	sf::Sprite highway(highwayTexture);
	highway.setScale({ scaleX, 1.f }); // stretch horizontally to match road width, keep vertical scale normal
	highway.setTextureRect(sf::IntRect({ 0, 0 }, { static_cast<int>(texSize.x), static_cast<int>(screenH) })); // use native width for the rect, not roadWidth
	highway.setPosition({highwaycenterX - highwayNewWidth / 2.f, 0.f });

	float highwayScrollOffset = 0.f;


	sf::Sprite highwayStatic(highwayTexture);
	highwayStatic.setTextureRect(sf::IntRect({ 0, 0 }, { static_cast<int>(texSize.x), static_cast<int>(screenH) }));
	highwayStatic.setScale({ scaleX, 1.f });
	highwayStatic.setPosition({ highwaycenterX- highwayNewWidth / 2.f, 0.f });

	////Road
	//sf::RectangleShape road({ roadWidth,600.f });
	//road.setFillColor(sf::Color(100, 100, 100));
	//road.setPosition({ roadX, 0.f });

	////Lane dashes
	//std::vector<sf::RectangleShape> laneDashes;
	//int numDashes = 12;
	//float dashH = 60.f;
	//float dashspacing = 150.f;

	//Obstacles
	std::vector<sf::Sprite> obstacles;
	std::vector<sf::Texture> obstacleTextures;
	std::vector<std::string> obstacleCarNames = {"carOne.png","carTwo.png","carThree.png","carFour.png","carFive.png","carSix.png",
												 //"carSeven.png",
												 "carEight.png","carNine.png","carTen.png","carEleven.png","carTwelve.png",
												 "carThirteen.png","carFourteen.png","carFifteen.png","carSixteen.png" };
	for (const std::string& name : obstacleCarNames) {
		sf::Texture texture;
		if(texture.loadFromFile("images/cars/" + name))
		{
			obstacleTextures.push_back(texture);
		}
		else
		{
			return -1;
		}
	}

	sf::Clock SpawnClock;
	float SpawnInterval = 1.2f; //spawn obstacle every 1.5 seconds
	float minspawnInterval = 0.3f; //minimum spawn interval
	float spawnDecreaseRate = 0.02f; //rate at which spawn interval decreases

	//background
	sf::Texture grassTexture;
	grassTexture.loadFromFile("images/background.png");
	grassTexture.setRepeated(true);	

	sf::Sprite background(grassTexture);
	background.setTextureRect(sf::IntRect({ 0,0 }, { static_cast<int>(screenW),static_cast<int>(screenH) }));

	float bgscrolloffset = 0.f;

	std::vector<sf::Texture> common;
	std::vector<std::string>commonNames = {
		"bush1",
		"bush2",
		"rock",
		"bush3",
		"bush4"
	};

	std::vector<sf::Texture> rare;
	std::vector<std::string>rareNames = {
	"dino1","dragon1","dragon2","dino2","dino3","dragon3","egg"
	};
	for (const std::string& name : commonNames)
	{
		sf::Texture texture;
		if (texture.loadFromFile("images/common/" + name + ".png"))
		{
			common.push_back(texture);
		}
		else
		{
			return -1;
		}
	}
	for (const std::string& name : rareNames)
	{
		sf::Texture texture;
		if (texture.loadFromFile("images/rare/" + name + ".png"))
		{
			rare.push_back(texture);
		}
		else
		{
			return -1;
		}
	}
	std::vector<DecorSprite> decorSprites;
	sf::Clock decorSpawnClock;
	float decorSpawnInterval = 2.5f; //spawn decor every 1.5 seconds

	//Speedometer HUD
	sf::Texture speedoTexture;
	speedoTexture.loadFromFile("images/speedometer.png");

	sf::Sprite speedoGauge(speedoTexture);
	float  speedoscale = 100.f / static_cast<float>(speedoTexture.getSize().x);
	speedoGauge.setScale({ speedoscale,speedoscale });
	speedoGauge.setPosition({ screenW - 120.f, screenH - 120.f });

	sf::RectangleShape speedoNeedle({ 4.f, 40.f });
	speedoNeedle.setFillColor(sf::Color::Red);
	speedoNeedle.setOrigin({ 2.f, 35.f }); // pivot point for rotation
	sf::FloatRect gaugeBounds = speedoGauge.getGlobalBounds();
	sf::Vector2f gaugeCenter = { gaugeBounds.position.x + gaugeBounds.size.x / 2.f,
								gaugeBounds.position.y + gaugeBounds.size.y / 2.f };
	speedoNeedle.setPosition(gaugeCenter);

	//Gameover Screen
	sf::RectangleShape overlay({ screenW,screenH });
	overlay.setFillColor(sf::Color(0, 0, 0, 200)); // Semi-transparent black

	sf::Text gameOverText(pixelFont);
	gameOverText.setString("GAME OVER");
	gameOverText.setCharacterSize(48);
	gameOverText.setFillColor(sf::Color::White);
	sf::FloatRect textBounds = gameOverText.getGlobalBounds();	
	gameOverText.setPosition({ screenW / 2.f - textBounds.size.x/2.f,180.f });

	sf::RectangleShape retryButton({ 160.f,50.f });
	retryButton.setFillColor(sf::Color::White);
	retryButton.setOutlineColor(sf::Color::Black);
	retryButton.setPosition({screenW/2.f-retryButton.getSize().x/2.f,320.f});

	sf::Text retryText(font);
	retryText.setString("Retry (R) ");
	retryText.setCharacterSize(20);
	retryText.setFillColor(sf::Color::Black);
	textBounds = retryText.getGlobalBounds();	
	retryText.setPosition({retryButton.getPosition().x + (retryButton.getSize().x-textBounds.size.x)/2.f,retryButton.getPosition().y + (retryButton.getSize().y-textBounds.size.y)/2.f});

	sf::RectangleShape exitButton({ 160.f,50.f });
	exitButton.setFillColor(sf::Color::White);
	exitButton.setOutlineColor(sf::Color::Black);	
	exitButton.setPosition({ screenW / 2.f - exitButton.getSize().x/2.f,380.f });

	sf::Text exitText(font);
	exitText.setString("Exit (Esc)");
	exitText.setCharacterSize(20);
	exitText.setFillColor(sf::Color::Black);
	textBounds = exitText.getGlobalBounds();
	exitText.setPosition({ exitButton.getPosition().x + (exitButton.getSize().x - textBounds.size.x) / 2.f,exitButton.getPosition().y + (exitButton.getSize().y - textBounds.size.y) / 2.f });


	//for (int i = 0;i < numDashes;i++) {
	//	sf::RectangleShape dash({ 10.f,dashH });
	//	dash.setFillColor(sf::Color::Yellow);
	//	dash.setPosition({ screenW / 2.f - 5.f,i * dashspacing });
	//	laneDashes.push_back(dash);//push_back is used to add the dash to the vector

	//}

	sf::Clock clock; //clock object to measure time

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				window.close();

			//Keyboard 
			if (const auto* keypressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keypressed->code == sf::Keyboard::Key::R) {
					resetGame(startSound,bgmusic, car, obstacles, score, roadspeed, SpawnInterval, speed, gameOver, startCarX, startCarY);
				}
				if (keypressed->code == sf::Keyboard::Key::Escape) {
					window.close();
				}
			}

			//Mouse
			if (const auto* mousepressed = event->getIf < sf::Event::MouseButtonPressed >()) {
				if (mousepressed->button == sf::Mouse::Button::Left) {
					sf::Vector2f mousePos = window.mapPixelToCoords(mousepressed->position);
					if (retryButton.getGlobalBounds().contains(mousePos)) {
						resetGame(startSound,bgmusic,car, obstacles, score, roadspeed, SpawnInterval, speed, gameOver, startCarX, startCarY);
					}
					if(exitButton.getGlobalBounds().contains(mousePos))
					{				
						window.close();
					}
				}
			}

		}
		float dt = clock.restart().asSeconds();

		if (!gameOver) {


			//Controls
			float speedSF = 0.5f; //player speed increase half the rate of the obstacles speed increase
			speed = BaseSpeed + (roadspeed - 200.f)*speedSF;

			sf::Vector2f movement(0.f, 0.f);
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
				movement.x -= speed * dt;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
				movement.x += speed * dt;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
				movement.y -= speed * dt;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
				movement.y += speed * dt;
			car.move(movement);

			//Boundary Maintain
			sf::Vector2f pos = car.getPosition();
			if (pos.x < roadX) pos.x = roadX;
			if (pos.x > roadX + roadWidth - carW)	pos.x = roadX + roadWidth - carW;
			if (pos.y < 0.f) pos.y = 0.f;
			if (pos.y > screenH - carH) pos.y = screenH - carH; //measuring from car's top side so 600(total side) -60(car's height)=540
			car.setPosition(pos);


		
			float speedIncreaseRate = 5.f;
			roadspeed += speedIncreaseRate * dt;
			score += roadspeed * dt * 0.1f; //0.1f is a scaling factor --> makes the score increase at a reasonable rate

			scoreText.setString("  Score : " + std::to_string(static_cast<int>(score)));
			highScoreText.setString("High Score\n       " + std::to_string(static_cast<int>(highscore)));

			//highway scrolling
			float overalapPX = 150.f;
			highwayScrollOffset -= roadspeed * dt/0.5f;
			if (highwayScrollOffset < 0.f) {
				highwayScrollOffset += static_cast<float>(highwayTexture.getSize().y);
			}

			sf::IntRect hRect = highway.getTextureRect();
			hRect.position.y = static_cast<int>(highwayScrollOffset);
			highway.setTextureRect(hRect);

			//background scrolling
			bgscrolloffset -= (roadspeed / (1.5f)) * dt;
			if (bgscrolloffset < 0.f) {
				bgscrolloffset += static_cast<float>(grassTexture.getSize().y);   //bgscrolloff.. += does is 
			}
			sf::IntRect bgRect = background.getTextureRect();
			bgRect.position.y = static_cast<int>(bgscrolloffset);
			background.setTextureRect(bgRect);
			
			//Speedometer needle rotation
			float minSpeed = 200.f;
			float maxSpeed = 600.f;
			float minAngle = -120.f;
			float maxAngle = 120.f;
			float speedRatio = (roadspeed - minSpeed) / (maxSpeed - minSpeed);
			//speedRatio = std::max(0.f, std::min(1.f, speedRatio)); // Clamp the ratio between 0 and 1
			speedRatio = std::clamp(speedRatio, 0.f, 1.f); //alternative way to clamp the ratio between 0 and 1
			float needleAngle = minAngle + speedRatio * (maxAngle - minAngle);
			speedoNeedle.setRotation(sf::degrees(needleAngle));

			//for (auto& dash : laneDashes){
			//	dash.move({ 0.f,roadspeed * dt }); 
			//	if (dash.getPosition().y > screenH) {
			//		dash.setPosition({ dash.getPosition().x,dash.getPosition().y - numDashes * dashspacing });
			//	}
			//}

			//obstacle spawning
			SpawnInterval -= spawnDecreaseRate * (dt/2.f);
			if (SpawnInterval < minspawnInterval) 
			{
				SpawnInterval = minspawnInterval;
			}

			if (SpawnClock.getElapsedTime().asSeconds() > SpawnInterval) {
				SpawnClock.restart();

				int laneIndex = std::rand() % numLanes; //random no from 0 to numLanes-1 (i.e 3)
				float obstacleX = getLaneX(laneIndex) + (laneW - carW) / 2.f; //center the obstacle in the lane

				int textureIndex = std::rand() % obstacleTextures.size();
				sf::Sprite obstacle(obstacleTextures[textureIndex]);
				sf::Vector2u obsTexSize = obstacleTextures[textureIndex].getSize();

				obstacle.setOrigin({ obsTexSize.x / 2.f, obsTexSize.y / 2.f });
				float unifromScale = carW / static_cast<float>(obsTexSize.x);
				obstacle.setScale({ unifromScale,unifromScale });

				if (laneIndex >= numLanes / 2)
				{
					obstacle.setRotation(sf::degrees(180.f)); //flip the obstacle horizontally
				}

				obstacle.setPosition({ obstacleX+carW/2.f,-carH/2.f+20.f});
				obstacles.push_back(obstacle);

			}
			for (auto& obstacle : obstacles) {
				obstacle.move({ 0.f,roadspeed * dt });
			}
			obstacles.erase(
				std::remove_if(obstacles.begin(), obstacles.end(), [](const sf::Sprite& obstacle) {
					return obstacle.getPosition().y > screenH+carH;}),
					obstacles.end());
		
			//collision checking
		
			for (auto& obstacle : obstacles) {
				if(checkCollision(car,obstacle))
				{
					gameOver = true;	
					gameoverSound.play();
					bgmusic.stop();
					if (score > highscore) {
						highscore = score;
						std::ofstream fout("highscore.txt");
						if (fout.is_open()){
						fout << highscore;
						fout.close();
						}
					}
				}
			}
			//if (gameOver) {
			//	car.setFillColor(sf::Color::White);
			//}
			//else {
			//	car.setFillColor(sf::Color::Blue);
			//}
			

			//background decor spawning
			if (decorSpawnClock.getElapsedTime().asSeconds() > decorSpawnInterval) {
				decorSpawnClock.restart();
				spawnDecor(decorSprites, common, rare);
			}

			for (auto& d : decorSprites) {
				d.sprite.move({ 0.f,(roadspeed/1.5f) * dt });
				d.lifetime += dt;
			}
			decorSprites.erase(
				std::remove_if(decorSprites.begin(),decorSprites.end(),[](const DecorSprite& d) {
					return d.sprite.getPosition().y > screenH + 50.f;
				}),
				decorSprites.end()
			);

		}

	window.clear(sf::Color::Black);
	window.draw(background);
	for (auto& d : decorSprites) {
		window.draw(d.sprite);
	}
	//window.draw(road);
	//for (auto& dash : laneDashes) {
	//	window.draw(dash);
	//}
	window.draw(highwayStatic);
	window.draw(highway);
	for (auto& obstacle : obstacles) {
		window.draw(obstacle);
	}
	window.draw(car);
	window.draw(scoreText);
	window.draw(highScoreText);
	window.draw(speedoGauge);
	window.draw(speedoNeedle);
	if (gameOver) {
		window.draw(overlay);
		window.draw(gameOverText);
		window.draw(retryButton);
		window.draw(retryText);
		window.draw(exitButton);
		window.draw(exitText);
	}
	window.display();
}

    return 0;
}