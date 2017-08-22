#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
int main()
{
    int windowX = 1000;
    int windowY = 600;
    // Inicializa a janela de desenho (800 x 600), com o título abaixo
    sf::RenderWindow window(sf::VideoMode(windowX, windowY), "Grand Theft Auto");

    // Cria uma forma ("shape") circular, com raio = 100
    sf::RectangleShape shape(sf::Vector2f(100, 20));
    shape.move(70,15);

    // Determina que a cor de preenchimento será verde
    shape.setFillColor(sf::Color::Red);

    sf::Texture texHeart;

    if(!texHeart.loadFromFile("data/img/heart.png"))

	{
		cout << "Erro na leitura da imagem!" << endl;
		exit(EXIT_FAILURE);
	}

    sf::Sprite imgHeart(texHeart);
    imgHeart.move(20,0);

    // DOLLAR

    sf::Texture texDollar;

    if(!texDollar.loadFromFile("data/img/dollar.png"))

	{
		cout << "Erro na leitura da imagem!" << endl;
		exit(EXIT_FAILURE);
	}

    sf::Sprite imgDollar(texDollar);
    imgDollar.setPosition(27.5,50);

    // DOLLAR

    // Cria e carrega a textura desejada
	sf::Texture tex;
	if(!tex.loadFromFile("data/img/car.png"))

	{
		cout << "Erro na leitura da imagem!" << endl;
		exit(EXIT_FAILURE);
	}
	// Cria um sprite, informa a textura e carrega o sprite no centro da janela
	sf::Sprite img(tex);
	img.setPosition(windowX/2, windowY/2);

	sf::Font font;
	if(!font.loadFromFile("data/fonts/dollar.ttf"))
    {
        cout << "Error" << endl;
    }

    sf::Text text;
    text.setFont(font);
    text.setColor(sf::Color::Green);
    text.setCharacterSize(18);
    text.setPosition(65, 55);

    int money = 0;

    // Enquanto a janela estiver aberta...
    while (window.isOpen())
    {
        sf::Event event;
        // Enquanto há eventos para processar...
        while (window.pollEvent(event))
        {
            // Se o usuário clicar no botão para fechar a janela, finaliza
            if (event.type == sf::Event::Closed)
                window.close();
            // Se o usuário pressionou ESC, finaliza
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();
        }

        sf::Vector2f pos = img.getPosition();
        sf::Vector2u size = window.getSize() - tex.getSize();
        // Seta para esquerda pressionada?
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            if (pos.x > 0) {
                img.move(-0.1,0);

            }
		}

		// Seta para esquerda pressionada?
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && pos.x < size.x) {
            if(pos.x < size.x)
                img.move(0.1,0);
		}

		// Seta para esquerda pressionada?
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && pos.y > 0) {
			if(pos.y > 0)
                img.move(0,-0.1);
		}

		// Seta para esquerda pressionada?
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && pos.y < size.y) {
			if(pos.y < size.y) {
                img.move(0,0.1);
                money++;
			}
		}

        // Limpa o conteúdo da janela
        window.clear();

        std::stringstream ss;
        ss << money;
        text.setString(ss.str());

        // Desenha os sprites
        window.draw(img);
        window.draw(shape);
        window.draw(imgHeart);
        window.draw(imgDollar);
        window.draw(text);

        // Exibe o conteúdo da janela na tela (double buffering)
        window.display();
    }

    return 0;
}
