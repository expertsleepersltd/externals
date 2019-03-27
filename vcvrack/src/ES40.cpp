#include "Encoders.hpp"

struct ModuleES40 : Module
{
	ModuleES40() : Module(0, 5, 2, 0) {}
	void step() override;
};

#define btf_n_factor (-(float)0x800000)
#define btf_p_factor ((float)0x800000)

#define ES_BITSTOFLOAT( bits )	\
( ( bits & 0x800000 ) ? ( (((float)(0xffffff&(-(int)(bits)))) / btf_n_factor ) ) : ( ((float)(bits)) / btf_p_factor ) )

void ModuleES40::step()
{
	int b[5];

	for ( int i=0; i<5; ++i )
	{
		float f = inputs[i].value;
		int bi = (int)f;
		b[i] = std::max( 0, std::min( 255, bi ) );
	}
	
    int bitsL = ( b[0] << 16 ) | ( b[1] << 8 ) | ( ( b[4] << 0 ) & 0xf0 );
	int bitsR = ( b[2] << 16 ) | ( b[3] << 8 ) | ( ( b[4] << 4 ) & 0xf0 );

	outputs[0].value = 10.0f * ES_BITSTOFLOAT( bitsL );
	outputs[1].value = 10.0f * ES_BITSTOFLOAT( bitsR );
}

struct ModuleES40Widget : ModuleWidget
{
	ModuleES40Widget(ModuleES40 *module) : ModuleWidget(module)
	{
		setPanel(SVG::load(assetPlugin(plugin, "res/ES40.svg")));

		for ( int i=0; i<5; ++i )
			addInput(Port::create<PJ301MPort>(Vec(17, 45+33*i), Port::INPUT, module, i));

		for ( int i=0; i<2; ++i )
			addOutput(Port::create<PJ301MPort>(Vec(17, (9+i)*33), Port::OUTPUT, module, i));
	}
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelES40 = Model::create<ModuleES40, ModuleES40Widget>( "Expert Sleepers", "ExpertSleepers-Encoders-ES40", "ES-4(0) Encoder", EXTERNAL_TAG );
