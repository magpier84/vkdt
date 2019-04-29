#include "pipe.h"

// TODO: for the local contrast op, maybe we'll want to split it like:
//
// [contr_beg] -> [tone curve] -> [contr_end]
//    \_______________________________/
// 
// for an arbitrary tone manipulating module in the middle, which will be
// run only on the brightness channel of the coarse residual.
// the temp buffer with all the pyramids required for detail reconstruction
// will need to be passed through on the side.
//

// TODO: we want simple descriptions here to autogenerate as much as we can of:
// 1) i/o specs: input/output/temp buffers, format, colour annotation
// 2) params: layout as uniforms, names when read from file/db
// 3) gui: simple annotations for params


// 1) maybe some textual description, for instance:
// * read only: main input
// * read only: context from preview pipe
// * main output
// * scratchpad memory
// we'll need to know:
// 
// * static: colour format and channel config
// * dynamic: size/dimensions
//
// the dynamic bits will need some api negotiation knowing required scratchpad
// memory for all modules in the graph and a get_roi_out() get_roi_in() kind of
// call chain. for non-linear dependency of scratchmem size on input size we may
// need some bisection (assuming monotonic dependency).
//
// R16UI  in  input    
// RGB16F in  context
// R32F   out output
// RGB32F tmp scratch

// as to 2):
// TODO: can we somehow simplify this into some preprocessor magic?
// TODO: we could put this into a text file with lines such as:
//
// detail :detail:1.0
// shadows:shadows:1.0
// hilight:highlights:1.0

// parameters to be translated and shown in gui:
const char *param_str =
{
  N_("detail"),
  N_("shadows"),
  N_("highlights"),
}
// default parameters:
float *param_def =
{
  1.0f, 1.0f, 1.0f,
};
// number of elements/floats per parameter: 
int *param_cnt =
{
  1, 1, 1,
}
// token id for storage backend and set/get functions:
dt_token_t param_tkn =
{
  dt_token("detail"),
  dt_token("shadows"),
  dt_token("hilight"),
};

// 3) TODO: also need one array for gui annotations (slider, colour picker, curve, ..)

// our private data:
typedef struct contrast_t
{
  // TODO: ideally we don't need this at all.
  // TODO: we should ask all our resources to be setup
  // TODO: when we create the pipeline. this includes other images,
  // TODO: mask channels, luts from disk, etc.
}
contrast_t;

// TODO: do we want to be called by token here?
// TODO: maybe the pipe core can do the reverse mapping
// TODO: for us and call us with param# instead (index in param_tkn array above).
// TODO: note that this number will be transient with module version and should
// TODO: not end up in backend storage.
//
// TODO: in fact ideally we would not be bothered by this at all, the core pipe
// TODO: would just push our new params in the above layout as push constants
// TODO: or uniform buffers for us. we'll need to provide an easy wrapper
// TODO: for the modules to create such arrays (above) and to query the uniform
// TODO: buffer in glsl/spirv.
void
set(dt_token_t param, const float *const val)
{
  if(param == dt_token("detail"))
    ; // ...
}

get(dt_token_t param)
{
}


// TODO: setup/teardown functions:
// need to create one VkPipeline per shader.
// need to instruct the core which buffers and descriptor sets we need.
// TODO: come up with nice wrapper functions here for concise use