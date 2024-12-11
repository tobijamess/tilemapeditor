#include "ui.h"
#include <iostream>

UI::UI(Editor& editor) : editor(editor) {}

// load the ui font, will be called when a new ui instance is initialized in editor
bool UI::Initialize() {
    if (!font.loadFromFile("assets/fonts/font.ttf")) {
        std::cerr << "Failed to load font: \n";
        return false;
    }
    return true;
}

void UI::HandleInteraction(const sf::Vector2f& mousePos, sf::RenderWindow& window) {
    for (const auto& button : buttons) {
        // check each buttons bounds to see if it contains the mouse position passed in from handle events
        if (button.shape.getGlobalBounds().contains(mousePos)) {
            // get the label text from each button
            std::string label = button.label.getString();
            // save the last clicked button if either save or load tilemap buttons were clicked
            if (label == "Save Tilemap" || label == "Load Tilemap") {
                lastClickedButton = label;
                ActivateTextInput();    // activate text input for saving or loading a tile map file
            }
            // depending on which label was on the pressed button, pass different layer dimensions to add layer function to create a new layer
            else if (label == "50x50 Grid") {
                editor.GetTileMap()->AddLayer(50, 50);
            }
            else if (label == "100x100 Grid") {
                editor.GetTileMap()->AddLayer(100, 100);
            }
            else if (label == "200x200 Grid") {
                editor.GetTileMap()->AddLayer(200, 200);
            }
            else if (label == "Merge Layers") {
                editor.GetTileMap()->showMergedLayers = !editor.GetTileMap()->showMergedLayers; // toggle showMergedLayers bool to true or false everytime button is pressed
                editor.GetTileMap()->MergeAllLayers(window, editor.GetTileMap()->showMergedLayers); // merge layers depending on the current bool state
            }
            std::cout << "Button clicked: " << label << "\n";
            break; // exit once the click was handled
        }
    }
}

void UI::DrawUI(sf::RenderWindow& window) {
    // populate the buttons vector if empty 
    if (buttons.empty()) {
        // define the properties of the UI buttons
        sf::Vector2f buttonSize(200.f, 100.f); // button dimensions
        float buttonSpacing = 20.f;          // spacing between buttons
        float startX = 20.f;                 // starting x position of buttons
        float startY = 20.f;                 // starting y position of buttons
        // define the buttons labels with dimensions since each button will create a new layer of that size
        std::vector<std::string> buttonLabels = {
            "50x50 Grid",
            "100x100 Grid",
            "200x200 Grid",
            "Merge Layers",
            "Save Tilemap",
            "Load Tilemap"
        };
        // iterate through the button labels vector and create buttons
        for (size_t i = 0; i < buttonLabels.size(); ++i) {
            // create and position the buttons with the properties defined above
            Button button;
            button.shape.setSize(buttonSize);
            button.shape.setFillColor(sf::Color(150, 150, 150));
            button.shape.setPosition(startX, startY + i * (buttonSize.y + buttonSpacing));
            // set the properties of the button label
            button.label.setFont(font);
            button.label.setString(buttonLabels[i]);
            button.label.setCharacterSize(24);
            button.label.setFillColor(sf::Color::Black);
            // center the label on the button
            sf::FloatRect textBounds = button.label.getLocalBounds();
            button.label.setPosition(
                button.shape.getPosition().x + (buttonSize.x - textBounds.width) / 2.f - textBounds.left,
                button.shape.getPosition().y + (buttonSize.y - textBounds.height) / 2.f - textBounds.top
            );
            // store the button in the ui buttons vector
            buttons.push_back(button);
        }
    }
    for (const auto& button : buttons) {
        // draw the button and its label
        window.draw(button.shape);
        window.draw(button.label);
    }
}

void UI::ActivateTextInput() {
    isTextInputActive = true;
    inputText.clear();
    // set up input box
    inputBox.setSize(sf::Vector2f(300.f, 50.f));
    inputBox.setFillColor(sf::Color(200, 200, 200));
    inputBox.setPosition(240.f, 480.f);
    // set up input text display
    inputTextDisplay.setFont(font);
    inputTextDisplay.setCharacterSize(20);
    inputTextDisplay.setFillColor(sf::Color::Black);
    inputTextDisplay.setPosition(inputBox.getPosition().x + 10.f, inputBox.getPosition().y + 10.f);
}

void UI::HandleTextInput(const sf::Event& event) {
    if (isTextInputActive) {
        if (event.type == sf::Event::TextEntered) {
            // handle text input (except special characters like backspace)
            if (event.text.unicode == '\b' && !inputText.empty()) {
                inputText.pop_back(); // handle Backspace
            }
            else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                inputText += static_cast<char>(event.text.unicode);
            }
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                // confirm input and deactivate text input mode
                isTextInputActive = false;
                std::cout << "Filename entered: " << inputText << "\n";
                // call save or load function
                if (lastClickedButton == "Save Tilemap") {
                    editor.GetTileMap()->SaveTileMap(inputText);
                }
                else if (lastClickedButton == "Load Tilemap") {
                    editor.GetTileMap()->LoadTileMap(inputText);
                }
            }
            else if (event.key.code == sf::Keyboard::Escape) {
                // cancel input
                isTextInputActive = false;
                inputText.clear();
            }
        }
    }
}

void UI::DrawTextInput(sf::RenderWindow& window) {
    if (isTextInputActive) {
        inputTextDisplay.setString(inputText);
        window.draw(inputBox);
        window.draw(inputTextDisplay);
    }
}