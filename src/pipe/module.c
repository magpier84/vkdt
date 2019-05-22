#include "module.h"
#include "graph.h"

// this is a public api function
int dt_module_add(
    dt_graph_t *graph,
    dt_token_t name,
    dt_token_t inst)
{
  // add to graph's list
  // make sure we have enough memory, realloc if not
  if(graph->num_modules == graph->max_modules-1)
  {
    assert(0 && "TODO: realloc module graph arrays");
    return -1;
  }
  const int modid = graph->num_modules++;

  dt_module_t *mod = graph->module + modid;
  mod->name = name;
  mod->inst = inst;

  // copy over initial info from module class:
  for(int i=0;i<dt_pipe.num_modules;i++)
  { // TODO: speed this up somehow?
    if(name == dt_pipe.module[i].name)
    {
      mod->so = dt_pipe.module + i;
      // TODO: slap over default params
      for(int p=0;p<mod->so->num_params;p++)
      {
        // XXX get definition and count and then set
        // TODO: need to alloc pointer + data from graph
      }
      // init connectors from our module class:
      for(int c=0;c<mod->so->num_connectors;c++)
      {
        mod->connector[c] = mod->so->connector[c];
        dt_connector_t *cn = mod->connector+c;
        // set connector's ref id's to -1 or ref count to 0 if a write|source node
        if(cn->type == dt_token("read") || cn->type == dt_token("sink"))
        {
          cn->connected_mid = -1;
          cn->connected_cid = -1;
        }
        else if(cn->type == dt_token("write") || cn->type == dt_token("source"))
        {
          cn->connected_mid = 0;
          cn->connected_cid = 0;
        }
      }
      mod->num_connectors = mod->so->num_connectors;
      break;
    }
  }
  // if connectors still empty fail
  if(mod->num_connectors == 0)
  {
    graph->num_modules--;
    return -1;
  }
  return modid;
}

int dt_module_get_connector(
    const dt_module_t *m, dt_token_t conn)
{
  for(int c=0;c<m->num_connectors;c++)
    if(m->connector[c].name == conn) return c;
  return -1;
}

int dt_module_remove(
    dt_graph_t *graph,
    const int modid)
{
  // TODO: mark as dead and increment dead counter. if we don't like it any
  // more, perform some form of garbage collection.
  assert(modid >= 0 && graph->num_modules > modid);
  // disconnect all channels
  for(int c=0;c<graph->module[modid].num_connectors;c++)
    if(dt_module_connect(graph, -1, -1, modid, c))
      return -1;
  graph->module[modid].name = 0;
  graph->module[modid].inst = 0;
  return 0;
}

// return modid or -1
int dt_module_get(
    const dt_graph_t *graph,
    dt_token_t name,
    dt_token_t inst)
{
  // TODO: some smarter way? hash table? benchmark how much we lose
  // through this stupid O(n) approach. after all the comparisons are
  // fast and we don't have all that many modules.
  for(int i=0;i<graph->num_modules;i++)
    if(graph->module[i].name == name &&
       graph->module[i].inst == inst)
      return i;
  return -1;
}
