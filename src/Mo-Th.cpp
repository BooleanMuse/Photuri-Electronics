#include "plugin.hpp"
#include <random>

struct Mo_Th : Module {
	enum ParamId {
		STEP1_PARAM,
		STEP2_PARAM,
		STEP3_PARAM,
		STEP4_PARAM,
		RANDOMNESS_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIGGER_INPUT,
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT1,
		OUTPUT2,
		OUTPUT3,
		OUTPUT4,
		OUT_NOTES,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};
	
	std::vector<int> pattern = {1, 2, 3, 2}; // Geometer Moth pattern
	std::vector<int> fibonacci = {1, 1, 2, 3, 5, 8, 13, 21};
    int step = 0;
	
	int currentStep = 0;
	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger resetTrigger;

	Mo_Th() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(STEP1_PARAM, 0.f, 1.f, 0.f, "Step Light 1");
		configParam(STEP2_PARAM, 0.f, 1.f, 0.f, "Step Light 2");
		configParam(STEP3_PARAM, 0.f, 1.f, 0.f, "Step Light 3");
		configParam(STEP4_PARAM, 0.f, 1.f, 0.f, "Step Light 4");

		configInput(TRIGGER_INPUT, "Trigger");
		configInput(RESET_INPUT, "Reset");
		
		configParam(RANDOMNESS_PARAM, 0.f, 1.f, 0.f, "Randomness");

		configOutput(OUTPUT1, "Moth 1");
		configOutput(OUTPUT2, "Moth 2");
		configOutput(OUTPUT3, "Moth 3");
		configOutput(OUTPUT4, "Moth 4");
		
		configOutput(OUT_NOTES, "Notes");
	}

	void process(const ProcessArgs& args) override {
		float clock = inputs[TRIGGER_INPUT].getVoltage();
		float reset = inputs[RESET_INPUT].getVoltage();
		float randomness = params[RANDOMNESS_PARAM].getValue();

		// Moth attraction to light behavior
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		float light = dis(gen);

		if (light > clock) {
			currentStep = (currentStep + 1) % 4;
			step = (step + 1) % pattern.size();
		}

		if (resetTrigger.process(reset)) {
			currentStep = 0;
		}

		// Set output voltages based on current step
		for (int i = 0; i < 4; i++) {
			float outputVoltage = params[i].getValue() * 10.0f;
			outputs[i].setVoltage(currentStep == i ? outputVoltage : 0.0f);
			
			
			  // Generate random note
			std::uniform_int_distribution<> noteDist(-12, 12);
			int randOffset = noteDist(gen) * randomness;
			int note = round(440.0 * pow(2, (double)(fibonacci[step] + randOffset - 69) / 12.0));
			// Output note value for current step
			outputs[OUT_NOTES].setVoltage((float)note / 12.0f);
		}
			
	}
};


struct MyCustomKnob : SvgKnob {
	MyCustomKnob() {
		maxAngle = 3.14159 * 2 * 0.8; // 80% of a circle
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobert.svg")));
		box.size = mm2px(Vec(20, 20));
	}
};

struct Mo_ThWidget : ModuleWidget {
	Mo_ThWidget(Mo_Th* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Mo-Th.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Add the step knobs to the widget
        addParam(createParamCentered<MyCustomKnob>(mm2px(Vec(23.876, 40)), module, Mo_Th::STEP1_PARAM));
        addParam(createParamCentered<MyCustomKnob>(mm2px(Vec(47.746, 40)), module, Mo_Th::STEP2_PARAM));
        addParam(createParamCentered<MyCustomKnob>(mm2px(Vec(71.622, 40)), module, Mo_Th::STEP3_PARAM));
        addParam(createParamCentered<MyCustomKnob>(mm2px(Vec(95.498, 40)), module, Mo_Th::STEP4_PARAM));
		
		addParam(createParamCentered<MyCustomKnob>(mm2px(Vec(60.95, 65.372)), module, Mo_Th::RANDOMNESS_PARAM));

        // Add the trigger and reset inputs to the widget
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.876, 65.372)), module, Mo_Th::TRIGGER_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(95.498, 65.372)), module, Mo_Th::RESET_INPUT));

        // Add the outputs to the widget
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.876, 90)), module, Mo_Th::OUTPUT1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.811, 90)), module, Mo_Th::OUTPUT2));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(83.56, 90)), module, Mo_Th::OUTPUT3));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(95.498, 90)), module, Mo_Th::OUTPUT4));
		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(60.95, 85)), module, Mo_Th::OUT_NOTES));
    
	}
};


Model* modelMo_Th = createModel<Mo_Th, Mo_ThWidget>("Mo-Th");