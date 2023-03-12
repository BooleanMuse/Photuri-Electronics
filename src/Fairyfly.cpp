#include "plugin.hpp"
#include <iostream>
#include <cmath>

struct Fairyfly : Module {
    enum ParamId {
		POS_PARAM,
		PITCH_PARAM,
		RANDOMNESS_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        PITCH_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        OUTPUT_1,
        OUTPUT_2,
		OUTPUT_3,
        OUTPUT_4,
        OUTPUT_5,
        OUTPUT_6,
		SIZE_OUTPUT,
		FAIRY_OUTPUT,
        OUTPUTS_LEN
    };
    

    float clockCounter = 0.f;
    int fireflyPosition = 0;
	int randomPosition = 0;

    Fairyfly() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN);
		configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "Pitch");
		configParam(RANDOMNESS_PARAM, 0.f, 1.f, 0.f, "Randomness");
		configParam(POS_PARAM, 0.f, 5.f, 0.f, "Position");
        configInput(PITCH_INPUT, "Clock");
        configOutput(OUTPUT_1, "Firefly 1");
        configOutput(OUTPUT_2, "Firefly 2");
        configOutput(OUTPUT_3, "Firefly 3");
        configOutput(OUTPUT_4, "Firefly 4");
        configOutput(OUTPUT_5, "Firefly 5");
        configOutput(OUTPUT_6, "Firefly 6");
		configOutput(SIZE_OUTPUT, "Fairyfly Size");
		configOutput(FAIRY_OUTPUT, "Fairyfly Wings");
    }

    void process(const ProcessArgs& args) override {
        // Read input pitch
        float pitch = params[PITCH_PARAM].getValue();
        pitch += inputs[PITCH_INPUT].getVoltage();
        pitch = clamp(pitch, 0.f, 10.f);
		float randomness = params[RANDOMNESS_PARAM].getValue();

        // Calculate firefly brightness
		float brightness = exp(-pitch);
		
		// Read position knob
        float position = (float)round(params[POS_PARAM].getValue());
        position = clamp(position, 0.f, 5.f);

        // Update clock counter and firefly position with randomization
		clockCounter += args.sampleTime;
		if (clockCounter >= powf(2, brightness * 8) / 10) {
			clockCounter -= powf(2, brightness * 8) / 10;
			int randOffset = (int)(random::uniform() * 6 * randomness);
			fireflyPosition = (fireflyPosition + randOffset + 1) % 6;
			randomPosition = (randomPosition + (int)round(random::uniform()) * 2 - 1) % 6;
		}

		// Output signals
		float outputVoltage = 0.f;
		switch (fireflyPosition) {
			case 0:
				outputVoltage = 0.1f;
				break;
			case 1:
				outputVoltage = 0.15f;
				break;
			case 2:
				outputVoltage = 0.2f;
				break;
			case 3:
				outputVoltage = 0.25f;
				break;
			case 4:
				outputVoltage = 0.3f;
				break;
			case 5:
				outputVoltage = 0.35f;
				break;
		}
		
		int outputRandom = 0;
		switch (randomPosition) {
			case 0:
				outputRandom = 1;
				break;
			case 1:
				outputRandom = 2;
				break;
			case 2:
				outputRandom = 3;
				break;
			case 3:
				outputRandom = 4;
				break;
			case 4:
				outputRandom = 5;
				break;
			case 5:
				outputRandom = 6;
				break;
		}

		outputVoltage *= brightness;		// Apply brightness to output voltage

		
		outputs[OUTPUT_1].setVoltage(outputVoltage *(fireflyPosition == 0 ? 50.f : 0.f));
		outputs[OUTPUT_2].setVoltage(outputVoltage *(fireflyPosition == 1 ? 50.f : 0.f));
		outputs[OUTPUT_3].setVoltage(outputVoltage *(fireflyPosition == 2 ? 50.f : 0.f));
		outputs[OUTPUT_4].setVoltage(outputVoltage *(fireflyPosition == 3 ? 50.f : 0.f));
		outputs[OUTPUT_5].setVoltage(outputVoltage *(fireflyPosition == 4 ? 50.f : 0.f));
		outputs[OUTPUT_6].setVoltage(outputVoltage *(fireflyPosition == 5 ? 50.f : 0.f));
		
		// Calculate and output size voltage
        float sizeVoltage = 0.f;
        if (position == 0) {
            sizeVoltage = 0.1f;
        } else {
            sizeVoltage = powf(10, -1.43f + 0.406f * position) / fireflyPosition;
        }
        outputs[SIZE_OUTPUT].setVoltage(sizeVoltage * (outputRandom));
		outputs[FAIRY_OUTPUT].setVoltage(sizeVoltage * (exp(-pitch)));
		
    }
};


struct MyCustomKnob : SvgKnob {
	MyCustomKnob() {
		maxAngle = 3.14159 * 2 * 0.8; // 80% of a circle
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobette.svg")));
		box.size = mm2px(Vec(20, 20));
	}
};

struct Knobairy : SvgKnob {
	Knobairy() {
		maxAngle = 3.14159 * 2 * 0.8; // 80% of a circle
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobairy.svg")));
		box.size = mm2px(Vec(20, 20));
	}
};

struct FairyflyWidget : ModuleWidget {
    FairyflyWidget(Fairyfly* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Fairyfly.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        /* addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 39.688)), module, Fairyfly::BRIGHTNESS_PARAM)); */
		
		addParam(createParamCentered<MyCustomKnob>(mm2px(Vec(22.86, 45)), module, Fairyfly::PITCH_PARAM));
		
		// Add firefly position knob
        addParam(createParamCentered<Knobairy>(mm2px(Vec(99.06, 40)), module, Fairyfly::POS_PARAM));
		
		addParam(createParamCentered<Knobairy>(mm2px(Vec(99.06, 60)), module, Fairyfly::RANDOMNESS_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.95, 45)), module, Fairyfly::PITCH_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 70)), module, Fairyfly::OUTPUT_1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.1, 70)), module, Fairyfly::OUTPUT_2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.34, 70)), module, Fairyfly::OUTPUT_3));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(68.58, 70)), module, Fairyfly::OUTPUT_4));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(83.82, 70)), module, Fairyfly::OUTPUT_5));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(99.06, 70)), module, Fairyfly::OUTPUT_6));
		
		// Add fairyfly-sized voltage output
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.1, 90)), module, Fairyfly::SIZE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(83.82, 90)), module, Fairyfly::FAIRY_OUTPUT));

        
    }
};


Model* modelFairyfly = createModel<Fairyfly, FairyflyWidget>("Fairyfly");
