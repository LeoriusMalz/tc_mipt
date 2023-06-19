#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include "sfline.hpp"
#include <iostream>

using std::cout, std::endl;

/*
    Icons from Ken111
    https://www.flaticon.com/ru/packs/game-skill?k=1650700359068

*/


class Node
{
public:
    Node(sf::Vector2f& position) 
        : mPosition{position}
    {
    }

    //virtual void draw(sf::RenderWindow& window) const = 0;
    //virtual bool onClick(sf::Vector2f mouseCoords) = 0;
    //virtual void onMousePressed(sf::Vector2f mouseCoords) = 0;

    enum class State
    {
        Blocked,
        Unblocked,
        Activated
    };


    void addChild(const std::shared_ptr<Node>& child)
    {
        mChildren.push_back(child);
    }

    sf::Vector2f getPosition()
    {
        return mPosition;
    }

    void unblock()
    {
        mState = State::Unblocked;
    }

    void block()
    {
        mState = State::Blocked;
        for (const auto& child : mChildren)
            child->block();
    }

    virtual bool collisionTest(sf::Vector2f mouseCoords) = 0;

    void onMousePressed(sf::Vector2f mouseCoords)
    {
        if (mState == State::Blocked)
            return;

        if (collisionTest(mouseCoords)) 
        {
            if (mState == State::Unblocked) 
            {
                mState = State::Activated;
                for (const auto& child : mChildren)
                    child->unblock();
            }

            else if (mState == State::Activated)
            {
                mState = State::Unblocked;
                for (const auto& child : mChildren)
                    child->block();
            }
        }

        for (const auto& child : mChildren)
        {
            child->onMousePressed(mouseCoords);
        }
    }
    virtual void draw(sf::RenderWindow& window) const = 0;

protected:

    sf::Vector2f mPosition {0, 0};
    State mState = State::Blocked;

    std::vector<std::shared_ptr<Node>> mChildren {};

    inline static sf::Color sBlockedColor   {40, 40, 40};
    inline static sf::Color sUnlockedColor  {80, 80, 40};
    inline static sf::Color sActivatedColor {160, 160, 40};
};


class HitNode : public Node
{
public:

    HitNode(sf::Vector2f position) 
        : Node{position}
    {
    }

    virtual sf::String getIconPath() = 0;

    void loadTexture()
    {
        sf::String texturePath = getIconPath();
        if (!mTexture.loadFromFile(texturePath))
        {
            cout << "Error! Can't load file " << texturePath.toAnsiString() << endl;
            std::exit(1);
        }
        mSprite.setTexture(mTexture);
        mSprite.setOrigin({mRadius, mRadius});
        mSprite.setPosition(mPosition);
    }


    sf::Color getCurrentColor() const
    {
        if (mState == State::Unblocked)
            return sUnlockedColor;
        else if (mState == State::Activated)
            return sActivatedColor;
        return sBlockedColor;
    }

    void draw(sf::RenderWindow& window) const
    {
        for (const auto& el : mChildren)
        {
            sfLine connectionLine {mPosition, el->getPosition(), getCurrentColor(), 2};
            connectionLine.draw(window);
            el->draw(window);
        }

        static sf::CircleShape shape(mRadius);
        shape.setOrigin({mRadius, mRadius});
        shape.setFillColor(getCurrentColor());
        shape.setPosition(mPosition);
        window.draw(shape);

        window.draw(mSprite);
    }

    bool collisionTest(sf::Vector2f mouseCoords) override
    {
        sf::Vector2f d = mPosition - mouseCoords;
        return d.x * d.x + d.y * d.y < mRadius * mRadius;
    }

class AccumulativeNode : public Node {
public:
    AccumulativeNode(sf::Vector2f position, int maxActivationLevel)
        : Node(position),
          mMaxActivationLevel(maxActivationLevel),
          mActivationLevel(0),
          mIsActivated(false),
          mIsBlocked(true)
    {
        if (!mFont.loadFromFile("arial.ttf")) {
            std::cerr << "Ошибка загрузки шрифта." << std::endl;
        }
        else {
            mLevelText.setFont(mFont);
            updateLevelText();
        }
    }

    void draw(sf::RenderWindow& window) {
        sf::RectangleShape shape(sf::Vector2f(50, 50));
        shape.setPosition(mPosition);
        shape.setFillColor(mIsActivated ? sf::Color::Green : sf::Color::Red);

        window.draw(shape);
        window.draw(mLevelText);
    }

    bool onClick(sf::Vector2f mouseCoords) {
        if (isMouseOver(mouseCoords)) {
            onMousePressed(mouseCoords);
            return true;
        }
        return false;
    }

    void onMousePressed(sf::Vector2f mouseCoords) {
        if (isMouseOver(mouseCoords)) {
            if (!mIsActivated) {
                activate();
            } else {
                decreaseActivationLevel();
                if (mActivationLevel == 0) {
                    deactivate();
                }
            }
        }
    }

private:
    sf::Text mLevelText;
    int mMaxActivationLevel;
    int mActivationLevel;
    bool mIsActivated;
    bool mIsBlocked;

    bool isMouseOver(sf::Vector2f mouseCoords) {
        sf::FloatRect bounds(mPosition, sf::Vector2f(50, 50));
        return bounds.contains(mouseCoords);
    }

    void activate() {
        mIsActivated = true;
        mIsBlocked = false;
        unlockChildren();
        updateLevelText();
    }

    void deactivate() {
        mIsActivated = false;
        mIsBlocked = true;
        lockChildren();
        updateLevelText();
    }

    void unlockChildren() {
        // Разблокировать всех потомков
    }

    void lockChildren() {
        // Заблокировать всех потомков
    }

    void decreaseActivationLevel() {
        mActivationLevel--;
        updateLevelText();
    }

    void updateLevelText() {
        std::stringstream ss;
        ss << mActivationLevel << " / " << mMaxActivationLevel;
        mLevelText.setString(ss.str());
        mLevelText.setCharacterSize(16);  // Размер текста (в пикселях)
        mLevelText.setFillColor(sf::Color::White);  // Цвет текста
    }

    sf::Font mFont; // Добавлено объявление переменной mFont
};

private:

    sf::Texture mTexture;
    sf::Sprite mSprite;

    float mRadius = 24;
    bool mIsActivated = false;
};



class BombSkillNode : public HitNode
{
public:
    BombSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_bomb.png"};
    }
};

class SpikesSkillNode : public HitNode
{
public:
    SpikesSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_spikes.png"};
    }
};


class LightningSkillNode : public HitNode
{
public:
    LightningSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_lightning.png"};
    }
};


class EyeSkillNode : public HitNode
{
public:
    EyeSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_eye.png"};
    }
};


class ClawsSkillNode : public HitNode
{
public:
    ClawsSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_claws.png"};
    }
};

class ShieldSkillNode : public HitNode
{
public:
    ShieldSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_shield.png"};
    }
};


class SwordSkillNode : public HitNode
{
public:
    SwordSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_sword.png"};
    }
};


class ShurikenSkillNode : public HitNode
{
public:
    ShurikenSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_shuriken.png"};
    }
};

class WindSkillNode : public HitNode
{
public:
    WindSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_shuriken.png"};
    }
};


class MeteoriteSkillNode : public HitNode
{
public:
    MeteoriteSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_meteorite.png"};
    }
};

class HandSkillNode : public HitNode
{
public:
    HandSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_hand.png"};
    }
};

class EarthquakeSkillNode : public HitNode
{
public:
    EarthquakeSkillNode(sf::Vector2f position) : HitNode{position} 
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_earthquake.png"};
    }
};

class FireballSkillNode : public HitNode
{
public:
    FireballSkillNode(sf::Vector2f position) : HitNode{position}
    {
        loadTexture();
    }

    sf::String getIconPath() override
    {
        return sf::String{"icons/icon_fireball.png"};
    }
};

std::shared_ptr<Node> createSkillTree()
{

    std::shared_ptr<Node> root  {new LightningSkillNode({400, 500})};
    std::shared_ptr<Node> a     {new ShurikenSkillNode({200, 400})};
    std::shared_ptr<Node> b     {new BombSkillNode({400, 400})};
    std::shared_ptr<Node> c     {new EyeSkillNode({600, 400})};
    root->addChild(a);
    root->addChild(b);
    root->addChild(c);

    a->addChild(std::shared_ptr<Node>{new ShieldSkillNode({100, 200})});
    a->addChild(std::shared_ptr<Node>{new EarthquakeSkillNode({300, 200})});

    std::shared_ptr<Node> f {new SwordSkillNode({200, 200})};
    a->addChild(f);
    f->addChild(std::shared_ptr<Node>{new FireballSkillNode({200, 100})});

    b->addChild(std::shared_ptr<Node>{new HandSkillNode({400, 200})});
    b->addChild(std::shared_ptr<Node>{new MeteoriteSkillNode({500, 200})});
    std::shared_ptr<Node> e {new BombSkillNode({600, 200})};
    b->addChild(e);
    e->addChild(std::shared_ptr<Node>{new WindSkillNode({500, 100})});
    e->addChild(std::shared_ptr<Node>{new SwordSkillNode({600, 100})});
    c->addChild(std::shared_ptr<Node>{new EyeSkillNode({700, 200})});

    return root;
}


int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Skill Tree", sf::Style::Close, settings);
    window.setFramerateLimit(60);

    std::shared_ptr<Node> root = createSkillTree();
    root->unblock();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();


            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f mouseCoords = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
                root->onMousePressed(mouseCoords);
            }
        }


        window.clear(sf::Color::Black);
        root->draw(window);
        window.display();
    }

    return 0;
}