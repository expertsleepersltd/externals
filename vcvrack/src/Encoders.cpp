#include "Encoders.hpp"

Plugin *plugin;

void init(Plugin *p)
{
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	p->addModel( model8GT );
	p->addModel( model8CV );
	p->addModel( modelES40 );
	p->addModel( modelES5 );
}
