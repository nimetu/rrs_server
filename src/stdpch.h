#include <nel/misc/types_nl.h>

#include <string>
#include <netinet/in.h>

#include <nel/misc/common.h>
#include <nel/misc/path.h>
#include <nel/misc/system_info.h>
#include <nel/misc/sheet_id.h>

#include <nel/misc/config_file.h>

#include <nel/net/service.h>
#include <nel/net/udp_sim_sock.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>

#include <nel/3d/u_play_list.h>
#include <nel/3d/u_play_list_manager.h>
#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_animation.h>

#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_transform.h>
//#include "nel/3d/skeleton_model.h"

#include <nel/3d/u_bone.h>

#include <nel/3d/u_particle_system_instance.h>
#include <nel/3d/particle_system_model.h>
#include <nel/3d/particle_system_shape.h>

#include <nel/misc/progress_callback.h>

#include "game_share/visual_slot_manager.h"
#include "game_share/gender.h"
#include "game_share/people_pd.h"
#include "game_share/player_visual_properties.h"

#include "game_share/slot_types.h"
#include "game_share/slot_equipment.h"
#include "game_share/item_type.h"

#include "client_sheets/race_stats_sheet.h"
//#include "client_sheets/character_sheet.h"
#include "sheet_manager.h"
#include "color_slot_manager.h"
#include "gabarit.h"

#ifdef NL_OS_WINDOWS
#define NOMINMAX
#include	<windows.h>
#endif // NL_OS_WINDOWS
