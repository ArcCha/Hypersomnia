#include "rendering_scripts.h"
#include "augs/drawing/drawing.h"
#include "augs/templates/visit_gettable.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sprite_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void draw_cast_spells_highlights(const draw_cast_spells_highlights_input in) {
	const auto& cosm = in.cosm;
	const auto dt = cosm.get_fixed_delta();

	cosm.for_each(
		processing_subjects::WITH_SENTIENCE,
		[&](const auto it) {
			const auto& sentience = it.get<components::sentience>();
			const auto casted_spell = sentience.currently_casted_spell;

			if (casted_spell.is_set()) {
				visit_gettable(
					sentience.spells,
					casted_spell,
					[&](const auto& spell){
						const auto highlight_amount = 1.f - (in.global_time_seconds - sentience.time_of_last_spell_cast.in_seconds(dt)) / 0.4f;

						if (highlight_amount > 0.f) {
							const auto spell_data = get_meta_of(spell, cosm.get_common_state().spells);

							auto highlight_col = spell_data.common.associated_color;
							highlight_col.a = static_cast<rgba_channel>(255 * highlight_amount);

							in.output.aabb_centered(
								in.cast_highlight_tex,
								in.camera[it.get_viewing_transform(in.interpolation).pos],
								highlight_col
							);
						}
					}
				);
			}
		}
	);
}