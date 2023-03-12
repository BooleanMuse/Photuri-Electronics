#include "plugin.hpp"


struct Cricketter : Module {
	
	float phase = 0.f;
	float blinkPhase = 0.f;
	
	enum ParamId {
		/* PITCH_PARAM, */
		CRICKET_MOD_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PITCH_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SINE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		BLINK_LIGHT,
		LIGHTS_LEN
	};

	Cricketter() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		/* configParam(PITCH_PARAM, 0.f, 1.f, 0.f, ""); */
		configParam(CRICKET_MOD_PARAM, 0.f, 10.f, 0.f, "Cricket Sound Modulation");
		configInput(PITCH_INPUT, "V/Oct");
		configOutput(SINE_OUTPUT, "Sound");
	}

	void process(const ProcessArgs& args) override {
				// Compute the frequency from the pitch parameter and input
		/* float pitch = params[PITCH_PARAM].getValue(); */
		/* pitch += inputs[PITCH_INPUT].getVoltage(); */
		float pitch = inputs[PITCH_INPUT].getVoltage();
		pitch = clamp(pitch, -4.f, 4.f);
		// The default pitch is C4 = 261.6256f
		float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);


		// Compute the cricket frequency
		float cricketFreq = 300.f; // Adjust as needed
		float cricketModParam = params[CRICKET_MOD_PARAM].getValue();
		float cricketMod = std::sin(2.f * M_PI * cricketModParam * phase);
		float cricket = std::sin(2.f * M_PI * (cricketFreq + cricketMod) * phase);

		// Accumulate the phase
		phase += freq * args.sampleTime;
		if (phase >= 0.5f)
			phase -= 1.f;

		// Compute the sine output with cicada and cricket sounds added
		float sine = std::sin(2.f * M_PI * phase)+ cricket;
		// Audio signals are typically +/-5V
		// https://vcvrack.com/manual/VoltageStandards
		outputs[SINE_OUTPUT].setVoltage(5.f * sine);

		// Blink light at 1Hz
		blinkPhase += args.sampleTime;
		if (blinkPhase >= 1.f)
			blinkPhase -= 1.f;
		lights[BLINK_LIGHT].setBrightness(blinkPhase < 0.5f ? 1.f : 0.f);
	}
};

struct MyCustomKnob : SvgKnob {
	MyCustomKnob() {
		maxAngle = 3.14159 * 2 * 0.8; // 80% of a circle
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knoby.svg")));
		box.size = mm2px(Vec(20, 20));
	}
};

struct CricketterWidget : ModuleWidget {
	CricketterWidget(Cricketter* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Cricketter.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		/* addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 46.063)), module, Cricketter::PITCH_PARAM)); */
		
		addParam(createParamCentered<MyCustomKnob>(mm2px(Vec(30.48, 65)), module, Cricketter::CRICKET_MOD_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 90)), module, Cricketter::PITCH_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.72, 90)), module, Cricketter::SINE_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.48, 36)), module, Cricketter::BLINK_LIGHT));
	}
};


Model* modelCricketter = createModel<Cricketter, CricketterWidget>("Cricketter");