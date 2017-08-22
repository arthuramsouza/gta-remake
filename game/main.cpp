#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;
int main()
{
    int windowX = 800;
    int windowY = 600;
    // Inicializa a janela de desenho (800 x 600), com o título abaixo
    sf::RenderWindow window(sf::VideoMode(windowX, windowY), "Grand Theft Auto");

    // Cria e carrega a textura desejada
	sf::Texture tex;
	if(!tex.loadFromFile("data/img/Car.png"))

	{
		cout << "Erro na leitura da imagem!" << endl;
		exit(EXIT_FAILURE);
	}
	// Cria um sprite, informa a textura e carrega o sprite no centro da janela
	sf::Sprite img(tex);
	img.setPosition(windowX/2, windowY/2);

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
			}
		}

        // Limpa o conteúdo da janela
        window.clear();

        // Desenha o sprite
        window.draw(img);

        // Exibe o conteúdo da janela na tela (double buffering)
        window.display();
    }

    return 0;
}
