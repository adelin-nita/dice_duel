#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "buton.h"

enum GameState
{
    MENU,
    PLAYING
};

struct Confetti
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float life;
    float rotation;
    float rotSpeed;
};

struct AmbientDust
{
    sf::Vector2f pos;
    float speed;
    float offset;
    float size;
    sf::Uint8 baseAlpha;
};

struct TrailNode
{
    sf::Vector2f pos;
    float scale;
};

// NOU: Structura pentru Textele Flotante (Pop-ups)
struct FloatingText
{
    std::string text;
    sf::Vector2f pos;
    sf::Color color;
    float life; // Scade de la 255 la 0
};

// NOU: Structura pentru Bumpers (Obstacole Pinball)
struct Bumper
{
    sf::Vector2f pos;
    float radius;
    float flashTimer; // Cat timp straluceste dupa ce e lovit
};

void rollDice(sf::Sprite &sprite1, sf::Sprite &sprite2, sf::Texture &t1, sf::Texture &t2, sf::Texture &t3, sf::Texture &t4, sf::Texture &t5, sf::Texture &t6, int &val1, int &val2)
{
    val1 = std::rand() % 6;
    val2 = std::rand() % 6;
    sf::Texture *tex[] = {&t1, &t2, &t3, &t4, &t5, &t6};
    sprite1.setTexture(*tex[val1]);
    sprite2.setTexture(*tex[val2]);
}

void triggerConfetti(std::vector<Confetti> &particule)
{
    for (int i = 0; i < 200; i++)
    {
        Confetti c;
        c.pos = sf::Vector2f(400.f, 400.f);
        c.vel = sf::Vector2f((std::rand() % 100 - 50) / 3.f, (std::rand() % 100 - 150) / 4.f);
        c.color = sf::Color(std::rand() % 255, std::rand() % 255, std::rand() % 255);
        c.life = 255.f;
        c.rotation = static_cast<float>(std::rand() % 360);
        c.rotSpeed = static_cast<float>((std::rand() % 20) - 10);
        particule.push_back(c);
    }
}

void drawTextWithShadow(sf::RenderWindow &window, sf::Text &text, sf::Color textColor)
{
    sf::Vector2f pos = text.getPosition();
    sf::Vector2f scale = text.getScale();
    text.setPosition(pos.x + 2.f * scale.x, pos.y + 2.f * scale.y);
    text.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(text);
    text.setPosition(pos);
    text.setFillColor(textColor);
    window.draw(text);
}

sf::RectangleShape createGlassPanel(sf::Vector2f size, sf::Vector2f position)
{
    sf::RectangleShape panel(size);
    panel.setPosition(position);
    panel.setFillColor(sf::Color(0, 0, 0, 140));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(255, 255, 255, 40));
    panel.setOrigin(size.x / 2.f, size.y / 2.f);
    return panel;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 800), "Dice Duel: Arcade Pinball Edition");
    window.setFramerateLimit(60);

    GameState currentState = MENU;

    sf::VertexArray gradientBackground(sf::Quads, 4);
    gradientBackground[0].position = sf::Vector2f(0.f, 0.f);
    gradientBackground[1].position = sf::Vector2f(800.f, 0.f);
    gradientBackground[2].position = sf::Vector2f(800.f, 800.f);
    gradientBackground[3].position = sf::Vector2f(0.f, 800.f);
    gradientBackground[0].color = sf::Color(25, 80, 45);
    gradientBackground[1].color = sf::Color(10, 40, 20);
    gradientBackground[2].color = sf::Color(5, 15, 10);
    gradientBackground[3].color = sf::Color(10, 40, 20);

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    bool hasBackground = false;
    if (backgroundTexture.loadFromFile("masa_cazinou.webp"))
    {
        backgroundSprite.setTexture(backgroundTexture);
        hasBackground = true;
    }

    sf::Font font;
    if (!font.loadFromFile("verdana.ttf"))
        return -1;
    sf::Image icon;
    if (icon.loadFromFile("download.jpg"))
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    std::vector<AmbientDust> prafMasa;
    for (int i = 0; i < 40; i++)
    {
        AmbientDust d;
        d.pos = sf::Vector2f(std::rand() % 800, std::rand() % 800);
        d.speed = (std::rand() % 10 + 5) / 30.f;
        d.offset = std::rand() % 100;
        d.size = (std::rand() % 3 + 1) * 1.0f;
        d.baseAlpha = std::rand() % 100 + 50;
        prafMasa.push_back(d);
    }

    sf::Text titluMeniu;
    titluMeniu.setFont(font);
    titluMeniu.setString("DICE DUEL");
    titluMeniu.setCharacterSize(80);
    sf::FloatRect titluRect = titluMeniu.getLocalBounds();
    titluMeniu.setOrigin(titluRect.width / 2.f, titluRect.height / 2.f);
    titluMeniu.setPosition(400.f, 200.f);

    sf::Text subtitluMeniu;
    subtitluMeniu.setFont(font);
    subtitluMeniu.setString("Pinball Edition");
    subtitluMeniu.setCharacterSize(24);
    sf::FloatRect subRect = subtitluMeniu.getLocalBounds();
    subtitluMeniu.setOrigin(subRect.width / 2.f, subRect.height / 2.f);
    subtitluMeniu.setPosition(400.f, 280.f);

    Button btnStartMeniu("Start Joc", {250, 60}, 20, sf::Color::White, sf::Color::Black);
    btnStartMeniu.setFont(font);
    btnStartMeniu.setPosition({275, 400});
    Button btnIesireMeniu("Iesire", {250, 60}, 20, sf::Color(220, 50, 50), sf::Color::White);
    btnIesireMeniu.setFont(font);
    btnIesireMeniu.setPosition({275, 500});
    Button butonJoc("Arunca Zarurile", {200, 50}, 16, sf::Color::White, sf::Color::Black);
    butonJoc.setFont(font);
    butonJoc.setPosition({300, 480});

    sf::Texture d1, d2, d3, d4, d5, d6;
    if (!d1.loadFromFile("zar_1.png") || !d2.loadFromFile("zar_2.png") || !d3.loadFromFile("zar_3.png") ||
        !d4.loadFromFile("zar_4.png") || !d5.loadFromFile("zar_5.png") || !d6.loadFromFile("zar_6.png"))
        return -1;

    std::srand(static_cast<unsigned int>(time(0)));
    sf::Sprite diceSprite, diceSprite1;
    int valZar1 = 0, valZar2 = 0;
    diceSprite.setTexture(d1);
    diceSprite1.setTexture(d1);
    diceSprite.setScale(0.4f, 0.4f);
    diceSprite1.setScale(0.4f, 0.4f);

    sf::FloatRect bounds1 = diceSprite.getLocalBounds();
    diceSprite.setOrigin(bounds1.width / 2.0f, bounds1.height / 2.0f);
    sf::FloatRect bounds2 = diceSprite1.getLocalBounds();
    diceSprite1.setOrigin(bounds2.width / 2.0f, bounds2.height / 2.0f);
    diceSprite.setPosition(300.f, 250.f);
    diceSprite1.setPosition(500.f, 250.f);

    bool mergeZarul = false;
    int numarulRotirilor = 0;
    int rotiriMaxime = 60;
    int shakeFrames = 0;
    sf::Vector2f velZar1(0.f, 0.f);
    sf::Vector2f velZar2(0.f, 0.f);
    bool glowOn = false;
    float inaltimeZar = 0.f;
    float currentZoom = 1.0f;
    float targetZoom = 1.0f;

    std::vector<TrailNode> trail1;
    std::vector<TrailNode> trail2;
    std::vector<FloatingText> popups; // Vector pentru texte flotante

    // NOU: Cream 3 Bumpers pe masa
    std::vector<Bumper> bumpers = {
        {{250.f, 200.f}, 30.f, 0.f},
        {{550.f, 200.f}, 30.f, 0.f},
        {{400.f, 320.f}, 40.f, 0.f} // Unul mare in centru
    };

    // Stare pentru ON FIRE
    bool playerOnFire = false;
    bool pcOnFire = false;

    // NOU: Logica fizicii acum include coliziunea cu bumpers
    auto applyPhysics = [&](sf::Sprite &sprite, sf::Vector2f &vel)
    {
        sf::Vector2f pos = sprite.getPosition();
        pos += vel;
        vel *= 0.95f;

        // Verificare lovire margini
        if (pos.x < 70.f)
        {
            pos.x = 70.f;
            vel.x *= -1.f;
        }
        if (pos.x > 730.f)
        {
            pos.x = 730.f;
            vel.x *= -1.f;
        }
        if (pos.y < 120.f)
        {
            pos.y = 120.f;
            vel.y *= -1.f;
        }
        if (pos.y > 380.f)
        {
            pos.y = 380.f;
            vel.y *= -1.f;
        }

        // Verificare lovire Bumpers (Pinball effect)
        for (auto &b : bumpers)
        {
            float dx = pos.x - b.pos.x;
            float dy = pos.y - b.pos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Raza zarului estimata la ~25px
            if (distance < b.radius + 25.f)
            {
                // Ricosam
                sf::Vector2f normal(dx / distance, dy / distance);
                vel = normal * (std::sqrt(vel.x * vel.x + vel.y * vel.y) * 1.5f); // Primeste un boost de viteza
                pos = b.pos + normal * (b.radius + 26.f);                         // Il scoatem afara din bumper
                b.flashTimer = 1.0f;                                              // Aprindem bumper-ul
                shakeFrames = 5;                                                  // Micul cutremur

                // Generam Text Arcade
                std::string bText = (std::rand() % 2 == 0) ? "BOING!" : "BAM!";
                popups.push_back({bText, pos, sf::Color::Yellow, 255.f});
            }
        }
        sprite.setPosition(pos);
    };

    std::vector<Confetti> particule;
    int scorJucator = 0, scorPC = 0;
    float afisajScorJucator = 0.f, afisajScorPC = 0.f;
    const int PUNCTE_TINTA = 50;
    bool randulJucatorului = true, gameOver = false;
    sf::Clock timerPC, globalClock;

    sf::RectangleShape panelTU = createGlassPanel({180.f, 50.f}, {120.f, 40.f});
    sf::RectangleShape panelPC = createGlassPanel({180.f, 50.f}, {680.f, 40.f});
    sf::RectangleShape panelMesaj = createGlassPanel({400.f, 80.f}, {400.f, 400.f});

    sf::Text textScorJucator, textScorPC, mesajZaruri, numele;
    textScorJucator.setFont(font);
    textScorJucator.setCharacterSize(22);
    textScorPC.setFont(font);
    textScorPC.setCharacterSize(22);
    mesajZaruri.setFont(font);
    mesajZaruri.setCharacterSize(20);
    mesajZaruri.setString("Primul la 50 de puncte castiga!\n   Dublele aduc puncte x2!");
    sf::FloatRect textRectInit = mesajZaruri.getLocalBounds();
    mesajZaruri.setOrigin(textRectInit.width / 2.0f, textRectInit.height / 2.0f);
    mesajZaruri.setPosition(400.0f, 400.0f);
    numele.setFont(font);
    numele.setCharacterSize(14);
    numele.setString("Realizat de: Nita Adelin-Alexandru");
    numele.setPosition(800.f - numele.getLocalBounds().width - 10.f, 770.f);

    // Text generic pentru a desena pop-up-urile
    sf::Text popText;
    popText.setFont(font);
    popText.setCharacterSize(20);

    while (window.isOpen())
    {
        float t = globalClock.getElapsedTime().asSeconds();
        float pulseFactor = (std::sin(t * 8.f) + 1.f) / 2.f;

        currentZoom += (targetZoom - currentZoom) * 0.05f;

        for (auto &d : prafMasa)
        {
            d.pos.y -= d.speed;
            d.pos.x += std::sin(t + d.offset) * 0.2f;
            if (d.pos.y < -10.f)
            {
                d.pos.y = 810.f;
                d.pos.x = std::rand() % 800;
            }
        }

        // Actualizare timer flash pentru Bumpere
        for (auto &b : bumpers)
        {
            if (b.flashTimer > 0)
                b.flashTimer -= 0.05f;
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.control && event.key.code == sf::Keyboard::W))
                window.close();

            if (currentState == MENU)
            {
                if (event.type == sf::Event::MouseMoved)
                {
                    btnStartMeniu.setBackColor(btnStartMeniu.isMouseOver(window) ? sf::Color(200, 255, 200) : sf::Color::White);
                    btnIesireMeniu.setBackColor(btnIesireMeniu.isMouseOver(window) ? sf::Color(255, 100, 100) : sf::Color(220, 50, 50));
                }
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    if (btnStartMeniu.isMouseOver(window))
                    {
                        currentState = PLAYING;
                        scorJucator = 0;
                        scorPC = 0;
                        afisajScorJucator = 0;
                        afisajScorPC = 0;
                        gameOver = false;
                        randulJucatorului = true;
                        particule.clear();
                        popups.clear();
                        glowOn = false;
                        trail1.clear();
                        trail2.clear();
                        inaltimeZar = 0.f;
                        playerOnFire = false;
                        pcOnFire = false;
                        butonJoc.setString("Arunca Zarurile");
                        mesajZaruri.setString("Sa inceapa meciul!");
                    }
                    if (btnIesireMeniu.isMouseOver(window))
                        window.close();
                }
            }
            else if (currentState == PLAYING)
            {
                if (event.type == sf::Event::MouseMoved)
                {
                    if ((randulJucatorului || gameOver) && butonJoc.isMouseOver(window))
                        butonJoc.setBackColor(sf::Color::Red);
                    else
                        butonJoc.setBackColor(randulJucatorului || gameOver ? sf::Color::White : sf::Color(150, 150, 150));
                }
                if (event.type == sf::Event::MouseButtonPressed && butonJoc.isMouseOver(window) && !mergeZarul)
                {
                    if (gameOver)
                    {
                        currentState = MENU;
                        targetZoom = 1.0f;
                    }
                    else if (randulJucatorului)
                    {
                        mergeZarul = true;
                        numarulRotirilor = 0;
                        mesajZaruri.setString("");
                        glowOn = false;

                        // Zarurile sunt mai rapide daca esti "ON FIRE"
                        float speedMult = playerOnFire ? 1.5f : 1.0f;
                        velZar1 = sf::Vector2f((std::rand() % 80) - 40.f, (std::rand() % 80) - 40.f) * speedMult;
                        velZar2 = sf::Vector2f((std::rand() % 80) - 40.f, (std::rand() % 80) - 40.f) * speedMult;

                        diceSprite.setPosition(350.f, 250.f);
                        diceSprite1.setPosition(450.f, 250.f);
                        trail1.clear();
                        trail2.clear();
                        targetZoom = 1.0f;
                    }
                }
            }
        }

        window.clear();
        if (hasBackground)
            window.draw(backgroundSprite);
        else
            window.draw(gradientBackground);

        sf::CircleShape prafShape;
        for (auto &d : prafMasa)
        {
            prafShape.setRadius(d.size);
            prafShape.setPosition(d.pos);
            sf::Uint8 currentAlpha = d.baseAlpha + static_cast<sf::Uint8>(std::sin(t * 3.f + d.offset) * 30.f);
            prafShape.setFillColor(sf::Color(255, 230, 150, currentAlpha));
            window.draw(prafShape);
        }

        if (currentState == MENU)
        {
            drawTextWithShadow(window, titluMeniu, sf::Color::Yellow);
            drawTextWithShadow(window, subtitluMeniu, sf::Color::White);
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && btnStartMeniu.isMouseOver(window))
                btnStartMeniu.setPosition({275, 404});
            else
                btnStartMeniu.setPosition({275, 400});
            btnStartMeniu.draw(window);
            btnIesireMeniu.draw(window);
            drawTextWithShadow(window, numele, sf::Color::White);
        }
        else if (currentState == PLAYING)
        {

            // DESENARE BUMPERS
            sf::CircleShape bumperShape;
            for (auto &b : bumpers)
            {
                bumperShape.setRadius(b.radius);
                bumperShape.setOrigin(b.radius, b.radius);
                bumperShape.setPosition(b.pos);

                // Miezul bumper-ului
                bumperShape.setFillColor(sf::Color(30, 30, 80));
                bumperShape.setOutlineThickness(4.f);

                // Strălucește (Neon) dacă e lovit
                if (b.flashTimer > 0)
                {
                    bumperShape.setOutlineColor(sf::Color(0, 255, 255, 255)); // Cyan aprins
                    bumperShape.setScale(1.1f, 1.1f);                         // Se mareste un pic cand e lovit
                }
                else
                {
                    bumperShape.setOutlineColor(sf::Color(100, 100, 150));
                    bumperShape.setScale(1.0f, 1.0f);
                }
                window.draw(bumperShape);
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && butonJoc.isMouseOver(window) && (randulJucatorului || gameOver))
                butonJoc.setPosition({300, 484});
            else
                butonJoc.setPosition({300, 480});

            if (!randulJucatorului && !mergeZarul && !gameOver)
            {
                if (timerPC.getElapsedTime().asSeconds() > 1.5f)
                {
                    mergeZarul = true;
                    numarulRotirilor = 0;
                    mesajZaruri.setString("");
                    glowOn = false;
                    targetZoom = 1.0f;

                    float speedMult = pcOnFire ? 1.5f : 1.0f;
                    velZar1 = sf::Vector2f((std::rand() % 80) - 40.f, (std::rand() % 80) - 40.f) * speedMult;
                    velZar2 = sf::Vector2f((std::rand() % 80) - 40.f, (std::rand() % 80) - 40.f) * speedMult;

                    diceSprite.setPosition(350.f, 250.f);
                    diceSprite1.setPosition(450.f, 250.f);
                    trail1.clear();
                    trail2.clear();
                }
            }

            float currentScale = 0.4f;
            float shadowDist = 8.f;

            if (mergeZarul && numarulRotirilor < rotiriMaxime)
            {
                numarulRotirilor++;
                int viteza = 1;
                if (numarulRotirilor > 40)
                    viteza = 3;
                if (numarulRotirilor > 50)
                    viteza = 6;
                if (numarulRotirilor % viteza == 0)
                {
                    rollDice(diceSprite, diceSprite1, d1, d2, d3, d4, d5, d6, valZar1, valZar2);
                    diceSprite.setRotation(static_cast<float>(std::rand() % 360));
                    diceSprite1.setRotation(static_cast<float>(-(std::rand() % 360)));
                }

                applyPhysics(diceSprite, velZar1);
                applyPhysics(diceSprite1, velZar2);

                float arcProgress = static_cast<float>(numarulRotirilor) / rotiriMaxime;
                inaltimeZar = 1.0f - std::pow((arcProgress - 0.5f) * 2.0f, 2.0f);
                currentScale = 0.4f + (inaltimeZar * 0.35f);
                shadowDist = 8.f + (inaltimeZar * 20.f);

                trail1.push_back({diceSprite.getPosition(), currentScale});
                if (trail1.size() > 8)
                    trail1.erase(trail1.begin());
                trail2.push_back({diceSprite1.getPosition(), currentScale});
                if (trail2.size() > 8)
                    trail2.erase(trail2.begin());
            }
            else if (numarulRotirilor >= rotiriMaxime && mergeZarul)
            {
                mergeZarul = false;
                inaltimeZar = 0.f;
                currentScale = 0.4f;
                shadowDist = 8.f;
                diceSprite.setRotation(0.f);
                diceSprite1.setRotation(0.f);
                shakeFrames = 10;
                trail1.clear();
                trail2.clear();

                targetZoom = 0.93f;

                int sumaZaruri = (valZar1 + 1) + (valZar2 + 1);
                bool eDubla = (valZar1 == valZar2);
                int puncteCastigate = sumaZaruri;
                std::string prefixMesaj = "";

                if (eDubla)
                {
                    puncteCastigate *= 2;
                    prefixMesaj = "DUBLA! Puncte x2!\n";
                    glowOn = true;
                    popups.push_back({"CRITICAL!", {400.f, 250.f}, sf::Color::Magenta, 255.f});
                }

                if (randulJucatorului)
                {
                    scorJucator += puncteCastigate;
                    if (sumaZaruri > 9)
                    {
                        playerOnFire = true;
                        popups.push_back({"ON FIRE!", {150.f, 250.f}, sf::Color::Red, 255.f});
                    }
                    else
                        playerOnFire = false;

                    if (scorJucator >= PUNCTE_TINTA)
                    {
                        gameOver = true;
                        mesajZaruri.setString("   VICTORIE!\nAi distrus PC-ul!");
                        butonJoc.setString("Inapoi in Meniu");
                        triggerConfetti(particule);
                        targetZoom = 0.9f;
                    }
                    else
                    {
                        randulJucatorului = false;
                        butonJoc.setString("PC-ul gandeste...");
                        mesajZaruri.setString(prefixMesaj + "Ai adunat " + std::to_string(puncteCastigate) + " pct.");
                        timerPC.restart();
                    }
                }
                else
                {
                    scorPC += puncteCastigate;
                    if (sumaZaruri > 9)
                    {
                        pcOnFire = true;
                        popups.push_back({"ON FIRE!", {650.f, 250.f}, sf::Color::Red, 255.f});
                    }
                    else
                        pcOnFire = false;

                    if (scorPC >= PUNCTE_TINTA)
                    {
                        gameOver = true;
                        mesajZaruri.setString("    AI PIERDUT...\nPC-ul a fost mai norocos.");
                        butonJoc.setString("Inapoi in Meniu");
                        targetZoom = 0.9f;
                    }
                    else
                    {
                        randulJucatorului = true;
                        butonJoc.setString("Arunca Zarurile");
                        mesajZaruri.setString(prefixMesaj + "PC a adunat " + std::to_string(puncteCastigate) + " pct.");
                    }
                }
                sf::FloatRect textRect = mesajZaruri.getLocalBounds();
                mesajZaruri.setOrigin(textRect.width / 2.0f, textRect.height / 2.0f);
                mesajZaruri.setPosition(400.0f, 400.0f);
            }

            diceSprite.setScale(currentScale, currentScale);
            diceSprite1.setScale(currentScale, currentScale);

            if (afisajScorJucator < scorJucator)
                afisajScorJucator += 0.3f;
            else
                afisajScorJucator = scorJucator;
            if (afisajScorPC < scorPC)
                afisajScorPC += 0.3f;
            else
                afisajScorPC = scorPC;

            textScorJucator.setString("TU: " + std::to_string((int)afisajScorJucator) + " / " + std::to_string(PUNCTE_TINTA));
            sf::FloatRect jRect = textScorJucator.getLocalBounds();
            textScorJucator.setOrigin(jRect.width / 2.f, jRect.height / 2.f);
            textScorJucator.setPosition(120.f, 35.f);
            textScorPC.setString("PC: " + std::to_string((int)afisajScorPC) + " / " + std::to_string(PUNCTE_TINTA));
            sf::FloatRect pcRect = textScorPC.getLocalBounds();
            textScorPC.setOrigin(pcRect.width / 2.f, pcRect.height / 2.f);
            textScorPC.setPosition(680.f, 35.f);

            sf::Color colorTU = playerOnFire ? sf::Color(255, 100, 0) : sf::Color::Cyan; // Portocaliu daca esti on fire
            if (afisajScorJucator < scorJucator)
            {
                colorTU = sf::Color::Yellow;
                textScorJucator.setScale(1.f, 1.f);
            }
            else if (scorJucator >= PUNCTE_TINTA - 10 && !gameOver)
            {
                colorTU = sf::Color(static_cast<sf::Uint8>(pulseFactor * 255), 255, 255);
                float animScale = 1.0f + (0.15f * pulseFactor);
                textScorJucator.setScale(animScale, animScale);
            }
            else
                textScorJucator.setScale(1.f, 1.f);

            sf::Color colorPC = pcOnFire ? sf::Color(255, 100, 0) : sf::Color::Red;
            if (afisajScorPC < scorPC)
            {
                colorPC = sf::Color::Yellow;
                textScorPC.setScale(1.f, 1.f);
            }
            else if (scorPC >= PUNCTE_TINTA - 10 && !gameOver)
            {
                colorPC = sf::Color(255, static_cast<sf::Uint8>(pulseFactor * 255), static_cast<sf::Uint8>(pulseFactor * 255));
                float animScale = 1.0f + (0.15f * pulseFactor);
                textScorPC.setScale(animScale, animScale);
            }
            else
                textScorPC.setScale(1.f, 1.f);

            sf::View view = window.getDefaultView();
            view.zoom(currentZoom);
            if (shakeFrames > 0)
            {
                float offsetX = static_cast<float>((std::rand() % 16) - 8);
                float offsetY = static_cast<float>((std::rand() % 16) - 8);
                view.move(offsetX, offsetY);
                shakeFrames--;
            }
            window.setView(view);

            // DESENARE DARE MICSORATE (Cu optiune de FOC)
            bool isActiveFire = (randulJucatorului && playerOnFire) || (!randulJucatorului && pcOnFire);
            sf::Color trailBaseColor = isActiveFire ? sf::Color(255, 80, 0) : sf::Color(255, 255, 255); // Foc vs Normal

            for (size_t i = 0; i < trail1.size(); i++)
            {
                sf::Sprite tSprite = diceSprite;
                tSprite.setPosition(trail1[i].pos);
                float tScale = trail1[i].scale * (0.3f + 0.7f * ((float)i / trail1.size()));
                tSprite.setScale(tScale, tScale);
                sf::Color c = trailBaseColor;
                c.a = static_cast<sf::Uint8>(i * 30);
                tSprite.setColor(c);
                window.draw(tSprite);
            }
            for (size_t i = 0; i < trail2.size(); i++)
            {
                sf::Sprite tSprite = diceSprite1;
                tSprite.setPosition(trail2[i].pos);
                float tScale = trail2[i].scale * (0.3f + 0.7f * ((float)i / trail2.size()));
                tSprite.setScale(tScale, tScale);
                sf::Color c = trailBaseColor;
                c.a = static_cast<sf::Uint8>(i * 30);
                tSprite.setColor(c);
                window.draw(tSprite);
            }

            sf::Vector2f pos1 = diceSprite.getPosition();
            diceSprite.setColor(sf::Color(0, 0, 0, 100));
            diceSprite.move(shadowDist, shadowDist);
            window.draw(diceSprite);
            diceSprite.setColor(sf::Color::White);
            diceSprite.setPosition(pos1);
            window.draw(diceSprite);

            sf::Vector2f pos2 = diceSprite1.getPosition();
            diceSprite1.setColor(sf::Color(0, 0, 0, 100));
            diceSprite1.move(shadowDist, shadowDist);
            window.draw(diceSprite1);
            diceSprite1.setColor(sf::Color::White);
            diceSprite1.setPosition(pos2);
            window.draw(diceSprite1);

            if (glowOn)
            {
                sf::RenderStates states;
                states.blendMode = sf::BlendAdd;
                sf::Sprite glow1 = diceSprite;
                glow1.setScale(currentScale + 0.08f, currentScale + 0.08f);
                glow1.setColor(sf::Color(255, 215, 0, 160));
                window.draw(glow1, states);
                sf::Sprite glow2 = diceSprite1;
                glow2.setScale(currentScale + 0.08f, currentScale + 0.08f);
                glow2.setColor(sf::Color(255, 215, 0, 160));
                window.draw(glow2, states);
            }

            // DESENARE POP-UPS ARCADE
            for (auto it = popups.begin(); it != popups.end();)
            {
                it->life -= 4.0f;  // Dispar incet
                it->pos.y -= 1.0f; // Se ridica in aer

                if (it->life <= 0)
                {
                    it = popups.erase(it);
                }
                else
                {
                    popText.setString(it->text);
                    sf::FloatRect r = popText.getLocalBounds();
                    popText.setOrigin(r.width / 2.f, r.height / 2.f);
                    popText.setPosition(it->pos);

                    sf::Color c = it->color;
                    c.a = static_cast<sf::Uint8>(it->life);
                    drawTextWithShadow(window, popText, c);
                    ++it;
                }
            }

            butonJoc.draw(window);
            window.draw(panelTU);
            window.draw(panelPC);
            if (mesajZaruri.getString() != "")
                window.draw(panelMesaj);
            drawTextWithShadow(window, mesajZaruri, gameOver ? (scorJucator >= PUNCTE_TINTA ? sf::Color::Green : sf::Color::Red) : sf::Color::Yellow);
            drawTextWithShadow(window, textScorJucator, colorTU);
            drawTextWithShadow(window, textScorPC, colorPC);
            drawTextWithShadow(window, numele, sf::Color::White);

            if (!particule.empty())
            {
                sf::RectangleShape rectConfetti(sf::Vector2f(10.f, 10.f));
                rectConfetti.setOrigin(5.f, 5.f);
                for (auto it = particule.begin(); it != particule.end();)
                {
                    it->vel.y += 0.3f;
                    it->pos += it->vel;
                    it->rotation += it->rotSpeed;
                    it->life -= 2.0f;
                    if (it->life <= 0)
                        it = particule.erase(it);
                    else
                    {
                        rectConfetti.setPosition(it->pos);
                        rectConfetti.setRotation(it->rotation);
                        sf::Color col = it->color;
                        col.a = static_cast<sf::Uint8>(std::max(0.f, it->life));
                        rectConfetti.setFillColor(col);
                        window.draw(rectConfetti);
                        ++it;
                    }
                }
            }
        }
        window.display();
    }
    return 0;
}
