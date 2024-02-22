// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2024 by Ronald "Eidolon" Kinard
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------

#include "g_gamedata.h"

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <filesystem>

#include <fmt/format.h>

#include "io/streams.hpp"
#include "d_main.h"
#include "m_argv.h"
#include "m_cond.h"
#include "g_game.h"
#include "r_skins.h"
#include "z_zone.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

void srb2::save_ng_gamedata()
{
	if (gamedata == NULL || !gamedata->loaded)
		return; // If never loaded (-nodata), don't save

	gamedata->deferredsave = false;

	if (usedCheats)
	{
#ifdef DEVELOP
		CONS_Alert(CONS_WARNING, M_GetText("Cheats used - Gamedata will not be saved.\n"));
#endif
		return;
	}

	GamedataJson ng {};

	ng.playtime.total = gamedata->totalplaytime;
	ng.rings.total = gamedata->totalrings;
	ng.playtime.tumble = gamedata->totaltumbletime;
	ng.rounds.race = gamedata->roundsplayed[GDGT_RACE];
	ng.rounds.battle = gamedata->roundsplayed[GDGT_BATTLE];
	ng.rounds.prisons = gamedata->roundsplayed[GDGT_PRISONS];
	ng.rounds.special = gamedata->roundsplayed[GDGT_SPECIAL];
	ng.rounds.custom = gamedata->roundsplayed[GDGT_CUSTOM];
	ng.challengekeys.pendingkeyrounds = gamedata->pendingkeyrounds;
	ng.challengekeys.pendingkeyroundoffset = gamedata->pendingkeyroundoffset;
	ng.challengekeys.keyspending = gamedata->keyspending;
	ng.challengekeys.chaokeys = gamedata->chaokeys;
	ng.milestones.everloadedaddon = gamedata->everloadedaddon;
	ng.milestones.everfinishcredits = gamedata->everfinishedcredits;
	ng.milestones.eversavedreplay = gamedata->eversavedreplay;
	ng.milestones.everseenspecial = gamedata->everseenspecial;
	ng.milestones.chaokeytutorial = gamedata->chaokeytutorial;
	ng.milestones.majorkeyskipattempted = gamedata->majorkeyskipattempted;
	ng.milestones.finishedtutorialchallenge = gamedata->finishedtutorialchallenge;
	ng.milestones.enteredtutorialchallenge = gamedata->enteredtutorialchallenge;
	ng.milestones.gonerlevel = gamedata->gonerlevel;
	ng.prisons.thisprisoneggpickup = gamedata->thisprisoneggpickup;
	ng.prisons.prisoneggstothispickup = gamedata->prisoneggstothispickup;
	ng.tafolderhash = quickncasehash(timeattackfolder, 64);
	ng.emblems.resize(MAXEMBLEMS, false);
	for (int i = 0; i < MAXEMBLEMS; i++)
	{
		ng.emblems[i] = gamedata->collected[i];
	}
	ng.unlockables.resize(MAXUNLOCKABLES, false);
	for (int i = 0; i < MAXUNLOCKABLES; i++)
	{
		ng.unlockables[i] = gamedata->unlocked[i];
	}
	ng.unlockpending.resize(MAXUNLOCKABLES, false);
	for (int i = 0; i < MAXUNLOCKABLES; i++)
	{
		ng.unlockpending[i] = gamedata->unlockpending[i];
	}
	ng.conditionsets.resize(MAXCONDITIONSETS, false);
	for (int i = 0; i < MAXCONDITIONSETS; i++)
	{
		ng.conditionsets[i] = gamedata->achieved[i];
	}
	if (gamedata->challengegrid)
	{
		ng.challengegrid.width = gamedata->challengegridwidth;
		ng.challengegrid.grid.resize(gamedata->challengegridwidth * CHALLENGEGRIDHEIGHT, 0);
		for (int i = 0; i < gamedata->challengegridwidth * CHALLENGEGRIDHEIGHT; i++)
		{
			ng.challengegrid.grid[i] = gamedata->challengegrid[i];
		}
	}
	ng.timesBeaten = gamedata->timesBeaten;
	for (int i = 0; i < numskins; i++)
	{
		srb2::GamedataSkinJson skin {};
		std::string name = std::string(skins[i].name);
		skin.records.wins = skins[i].records.wins;
		ng.skins[name] = std::move(skin);
	}
	for (auto unloadedskin = unloadedskins; unloadedskin; unloadedskin = unloadedskin->next)
	{
		srb2::GamedataSkinJson skin {};
		std::string name = std::string(unloadedskin->name);
		skin.records.wins = unloadedskin->records.wins;
		ng.skins[name] = std::move(skin);
	}
	for (int i = 0; i < nummapheaders; i++)
	{
		srb2::GamedataMapJson map {};
		std::string lumpname = std::string(mapheaderinfo[i]->lumpname);
		map.visited.visited = mapheaderinfo[i]->records.mapvisited & MV_VISITED;
		map.visited.beaten = mapheaderinfo[i]->records.mapvisited & MV_BEATEN;
		map.visited.encore = mapheaderinfo[i]->records.mapvisited & MV_ENCORE;
		map.visited.spbattack = mapheaderinfo[i]->records.mapvisited & MV_SPBATTACK;
		map.visited.mysticmelody = mapheaderinfo[i]->records.mapvisited & MV_MYSTICMELODY;
		map.stats.timeattack.besttime = mapheaderinfo[i]->records.time;
		map.stats.timeattack.bestlap = mapheaderinfo[i]->records.lap;
		map.stats.spbattack.besttime = 0;
		map.stats.spbattack.bestlap = 0;
		ng.maps[lumpname] = std::move(map);
	}
	for (auto unloadedmap = unloadedmapheaders; unloadedmap; unloadedmap = unloadedmap->next)
	{
		srb2::GamedataMapJson map {};
		std::string lumpname = std::string(unloadedmap->lumpname);
		map.visited.visited = unloadedmap->records.mapvisited & MV_VISITED;
		map.visited.beaten = unloadedmap->records.mapvisited & MV_BEATEN;
		map.visited.encore = unloadedmap->records.mapvisited & MV_ENCORE;
		map.visited.spbattack = unloadedmap->records.mapvisited & MV_SPBATTACK;
		map.visited.mysticmelody = unloadedmap->records.mapvisited & MV_MYSTICMELODY;
		map.stats.timeattack.besttime = unloadedmap->records.time;
		map.stats.timeattack.bestlap = unloadedmap->records.lap;
		map.stats.spbattack.besttime = 0;
		map.stats.spbattack.bestlap = 0;
		ng.maps[lumpname] = std::move(map);
	}
	for (int i = 0; i < gamedata->numspraycans; i++)
	{
		srb2::GamedataSprayCanJson spraycan {};

		candata_t* can = &gamedata->spraycans[i];

		if (can->col >= numskincolors)
		{
			continue;
		}
		spraycan.color = std::string(skincolors[can->col].name);

		if (can->map == NEXTMAP_INVALID)
		{
			spraycan.map = "";
			ng.spraycans.emplace_back(std::move(spraycan));
			continue;
		}

		if (can->map >= nummapheaders)
		{
			continue;
		}

		mapheader_t* mapheader = mapheaderinfo[can->map];
		if (!mapheader)
		{
			continue;
		}
		spraycan.map = std::string(mapheader->lumpname);
		ng.spraycans.emplace_back(std::move(spraycan));
	}
	for (auto cup = kartcupheaders; cup; cup = cup->next)
	{
		if (cup->windata[0].best_placement == 0)
		{
			continue;
		}
		srb2::GamedataCupJson cupdata {};
		cupdata.name = std::string(cup->name);
		for (size_t i = 0; i < std::min<size_t>(KARTGP_MAX, cupdata.records.size()); i++)
		{
			cupdata.records[i].bestgrade = cup->windata[i].best_grade;
			cupdata.records[i].bestplacement = cup->windata[i].best_placement;
			cupdata.records[i].bestskin = std::string(skins[cup->windata[i].best_skin.id].name);
			cupdata.records[i].gotemerald = cup->windata[i].got_emerald;
		}
		ng.cups[cupdata.name] = std::move(cupdata);
	}
	for (auto unloadedcup = unloadedcupheaders; unloadedcup; unloadedcup = unloadedcup->next)
	{
		if (unloadedcup->windata[0].best_placement == 0)
		{
			continue;
		}
		srb2::GamedataCupJson cupdata {};
		cupdata.name = std::string(unloadedcup->name);
		for (int i = 0; i < 4; i++)
		{
			cupdata.records[i].bestgrade = unloadedcup->windata[i].best_grade;
			cupdata.records[i].bestplacement = unloadedcup->windata[i].best_placement;
			cupdata.records[i].bestskin = std::string(skins[unloadedcup->windata[i].best_skin.id].name);
			cupdata.records[i].gotemerald = unloadedcup->windata[i].got_emerald;
		}
		ng.cups[cupdata.name] = std::move(cupdata);
	}

	std::string gamedataname_s {gamedatafilename};
	fs::path savepath {fmt::format("{}/{}", srb2home, gamedataname_s)};
	fs::path tmpsavepath {fmt::format("{}/{}.tmp", srb2home, gamedataname_s)};

	json ngdata_json = ng;

	try
	{
		std::string tmpsavepathstring = tmpsavepath.string();
		srb2::io::FileStream file {tmpsavepathstring, srb2::io::FileStreamMode::kWrite};
		srb2::io::BufferedOutputStream<srb2::io::FileStream> bos {std::move(file)};

		// The header is necessary to validate during loading.
		srb2::io::write(static_cast<uint32_t>(0xBA5ED321), bos); // major
		srb2::io::write(static_cast<uint8_t>(0), bos); // minor/flags
		srb2::io::write(static_cast<uint8_t>(gamedata->evercrashed), bos); // dirty (crash recovery)

		std::vector<uint8_t> ubjson = json::to_ubjson(ng);
		srb2::io::write_exact(bos, tcb::as_bytes(tcb::make_span(ubjson)));
		bos.flush();
		file = bos.stream();
		file.close();
	}
	catch (const srb2::io::FileStreamException& ex)
	{
		CONS_Alert(CONS_ERROR, "NG Gamedata save failed: %s\n", ex.what());
	}
	catch (...)
	{
		CONS_Alert(CONS_ERROR, "NG Gamedata save failed\n");
	}

	try
	{
		// Now that the save is written successfully, move it over the old save
		fs::rename(tmpsavepath, savepath);
	}
	catch (const fs::filesystem_error& ex)
	{
		CONS_Alert(CONS_ERROR, "NG Gamedata save succeeded but did not replace old save successfully: %s\n", ex.what());
	}
}

// G_SaveGameData
// Saves the main data file, which stores information such as emblems found, etc.
void G_SaveGameData(void)
{
	try
	{
		srb2::save_ng_gamedata();
	}
	catch (...)
	{
		CONS_Alert(CONS_ERROR, "Gamedata save failed\n");
		return;
	}

	// Also save profiles here.
	PR_SaveProfiles();

	#ifdef DEVELOP
		CONS_Alert(CONS_NOTICE, M_GetText("Gamedata saved.\n"));
	#endif
}

static const char *G_GameDataFolder(void)
{
	if (strcmp(srb2home,"."))
		return srb2home;
	else
		return "the Ring Racers folder";
}

void srb2::load_ng_gamedata()
{
	// Stop saving, until we successfully load it again.
	gamedata->loaded = false;

	// Clear things so previously read gamedata doesn't transfer
	// to new gamedata
	// see also M_EraseDataResponse
	G_ClearRecords(); // records
	M_ClearStats(); // statistics
	M_ClearSecrets(); // emblems, unlocks, maps visited, etc

	if (M_CheckParm("-nodata"))
	{
		// Don't load at all.
		// The following used to be in M_ClearSecrets, but that was silly.
		M_UpdateUnlockablesAndExtraEmblems(false, true);
		M_FinaliseGameData();
		gamedata->loaded = true;
		return;
	}

	if (M_CheckParm("-resetdata"))
	{
		// Don't load, but do save. (essentially, reset)
		M_FinaliseGameData();
		gamedata->loaded = true;
		return;
	}

	std::string datapath {fmt::format("{}/{}", srb2home, gamedatafilename)};

	srb2::io::BufferedInputStream<srb2::io::FileStream> bis;
	try
	{
		srb2::io::FileStream file {datapath, srb2::io::FileStreamMode::kRead };
		bis = srb2::io::BufferedInputStream(std::move(file));
	}
	catch (const srb2::io::FileStreamException& ex)
	{
		M_FinaliseGameData();
		gamedata->loaded = true;
		return;
	}

	uint32_t majorversion;
	uint8_t minorversion;
	uint8_t dirty;
	try
	{
		majorversion = srb2::io::read_uint32(bis);
		minorversion = srb2::io::read_uint8(bis);
		dirty = srb2::io::read_uint8(bis);
	}
	catch (...)
	{
		CONS_Alert(CONS_ERROR, "Failed to read ng gamedata header\n");
		return;
	}

	if (majorversion != 0xBA5ED321)
	{
		const char* gdfolder = G_GameDataFolder();
		I_Error("Game data is not for Ring Racers v2.0.\nDelete %s (maybe in %s) and try again.", gamedatafilename, gdfolder);
		return;
	}

	std::vector<std::byte> remainder = srb2::io::read_to_vec(bis);

	GamedataJson js;
	try
	{
		// safety: std::byte repr is always uint8_t 1-byte aligned
		tcb::span<uint8_t> remainder_as_u8 = tcb::span((uint8_t*)remainder.data(), remainder.size());
		json parsed = json::from_ubjson(remainder_as_u8);
		js = parsed.template get<GamedataJson>();
	}
	catch (...)
	{
		const char* gdfolder = G_GameDataFolder();
		I_Error("Game data is corrupt.\nDelete %s (maybe in %s) and try again.", gamedatafilename, gdfolder);
		return;
	}

	// Quick & dirty hash for what mod this save file is for.
	if (js.tafolderhash != quickncasehash(timeattackfolder, 64))
	{
		const char* gdfolder = G_GameDataFolder();
		I_Error("Game data is corrupt.\nDelete %s (maybe in %s) and try again.", gamedatafilename, gdfolder);
		return;
	}

	// Now we extract the json struct's data and put it into the C-side gamedata.

	gamedata->evercrashed = dirty;

	gamedata->totalplaytime = js.playtime.total;
	gamedata->totalrings = js.rings.total;
	gamedata->totaltumbletime = js.playtime.tumble;
	gamedata->roundsplayed[GDGT_RACE] = js.rounds.race;
	gamedata->roundsplayed[GDGT_BATTLE] = js.rounds.battle;
	gamedata->roundsplayed[GDGT_PRISONS] = js.rounds.prisons;
	gamedata->roundsplayed[GDGT_SPECIAL] = js.rounds.special;
	gamedata->roundsplayed[GDGT_CUSTOM] = js.rounds.custom;
	gamedata->pendingkeyrounds = js.challengekeys.pendingkeyrounds;
	gamedata->pendingkeyroundoffset = js.challengekeys.pendingkeyroundoffset;
	gamedata->keyspending = js.challengekeys.keyspending;
	gamedata->chaokeys = js.challengekeys.chaokeys;
	gamedata->everloadedaddon = js.milestones.everloadedaddon;
	gamedata->everfinishedcredits = js.milestones.everfinishcredits;
	gamedata->eversavedreplay = js.milestones.eversavedreplay;
	gamedata->everseenspecial = js.milestones.everseenspecial;
	gamedata->chaokeytutorial = js.milestones.chaokeytutorial;
	gamedata->majorkeyskipattempted = js.milestones.majorkeyskipattempted;
	gamedata->finishedtutorialchallenge = js.milestones.finishedtutorialchallenge;
	gamedata->enteredtutorialchallenge = js.milestones.enteredtutorialchallenge;
	gamedata->gonerlevel = js.milestones.gonerlevel;
	gamedata->thisprisoneggpickup = js.prisons.thisprisoneggpickup;
	gamedata->prisoneggstothispickup = js.prisons.prisoneggstothispickup;

	size_t emblems_size = js.emblems.size();
	for (size_t i = 0; i < std::min((size_t)MAXEMBLEMS, emblems_size); i++)
	{
		gamedata->collected[i] = js.emblems[i];
	}

	size_t unlocks_size = js.unlockables.size();
	for (size_t i = 0; i < std::min((size_t)MAXUNLOCKABLES, unlocks_size); i++)
	{
		gamedata->unlocked[i] = js.unlockables[i];
	}

	size_t pending_unlocks_size = js.unlockpending.size();
	for (size_t i = 0; i < std::min((size_t)MAXUNLOCKABLES, pending_unlocks_size); i++)
	{
		gamedata->unlockpending[i] = js.unlockpending[i];
	}

	size_t conditions_size = js.conditionsets.size();
	for (size_t i = 0; i < std::min((size_t)MAXCONDITIONSETS, conditions_size); i++)
	{
		gamedata->achieved[i] = js.conditionsets[i];
	}

	if (M_CheckParm("-resetchallengegrid"))
	{
		gamedata->challengegridwidth = 0;
		if (gamedata->challengegrid)
		{
			Z_Free(gamedata->challengegrid);
			gamedata->challengegrid = nullptr;
		}
	}
	else
	{
		gamedata->challengegridwidth = std::min(js.challengegrid.width, (uint32_t)0);
		if (gamedata->challengegrid)
		{
			Z_Free(gamedata->challengegrid);
			gamedata->challengegrid = nullptr;
		}
		if (gamedata->challengegridwidth)
		{
			gamedata->challengegrid = static_cast<uint16_t*>(Z_Malloc(
				(gamedata->challengegridwidth * CHALLENGEGRIDHEIGHT * sizeof(UINT16)),
				PU_STATIC, NULL));
			for (size_t i = 0; i < std::min((size_t)(gamedata->challengegridwidth * CHALLENGEGRIDHEIGHT), js.challengegrid.grid.size()); i++)
			{
				int16_t gridvalue = js.challengegrid.grid[i];
				if (gridvalue < 0)
				{
					gamedata->challengegrid[i] = MAXUNLOCKABLES;
				}
				else
				{
					gamedata->challengegrid[i] = static_cast<uint8_t>(gridvalue);
				}
			}

			M_SanitiseChallengeGrid();
		}
		else
		{
			gamedata->challengegrid = nullptr;
		}
	}

	gamedata->timesBeaten = js.timesBeaten;

	// Main records

	for (auto& skinpair : js.skins)
	{
		INT32 skin = R_SkinAvailable(skinpair.first.c_str());
		skinrecord_t dummyrecord {};
		dummyrecord.wins = skinpair.second.records.wins;

		if (skin != -1)
		{
			skins[skin].records = dummyrecord;
		}
		else if (dummyrecord.wins)
		{
			// Invalid, but we don't want to lose all the juicy statistics.
			// Instead, update a FILO linked list of "unloaded skins".

			unloaded_skin_t *unloadedskin =
				static_cast<unloaded_skin_t*>(Z_Malloc(
					sizeof(unloaded_skin_t),
					PU_STATIC, NULL
				));

			// Establish properties, for later retrieval on file add.
			strlcpy(unloadedskin->name, skinpair.first.c_str(), sizeof(unloadedskin->name));
			unloadedskin->namehash = quickncasehash(unloadedskin->name, SKINNAMESIZE);

			// Insert at the head, just because it's convenient.
			unloadedskin->next = unloadedskins;
			unloadedskins = unloadedskin;

			// Finally, copy into.
			unloadedskin->records = dummyrecord;
		}
	}

	for (auto& mappair : js.maps)
	{
		UINT16 mapnum = G_MapNumber(mappair.first.c_str());
		recorddata_t dummyrecord {};
		dummyrecord.mapvisited |= mappair.second.visited.visited ? MV_VISITED : 0;
		dummyrecord.mapvisited |= mappair.second.visited.beaten ? MV_BEATEN : 0;
		dummyrecord.mapvisited |= mappair.second.visited.encore ? MV_ENCORE : 0;
		dummyrecord.mapvisited |= mappair.second.visited.spbattack ? MV_SPBATTACK : 0;
		dummyrecord.mapvisited |= mappair.second.visited.mysticmelody ? MV_SPBATTACK : 0;
		dummyrecord.time = mappair.second.stats.timeattack.besttime;
		dummyrecord.lap = mappair.second.stats.timeattack.bestlap;

		if (mapnum < nummapheaders && mapheaderinfo[mapnum])
		{
			// Valid mapheader, time to populate with record data.

			mapheaderinfo[mapnum]->records = dummyrecord;
		}
		else if (dummyrecord.mapvisited & MV_BEATEN || dummyrecord.time != 0 || dummyrecord.lap != 0)
		{
			// Invalid, but we don't want to lose all the juicy statistics.
			// Instead, update a FILO linked list of "unloaded mapheaders".

			unloaded_mapheader_t *unloadedmap =
				static_cast<unloaded_mapheader_t*>(Z_Malloc(
					sizeof(unloaded_mapheader_t),
					PU_STATIC, NULL
				));

			// Establish properties, for later retrieval on file add.
			unloadedmap->lumpname = Z_StrDup(mappair.first.c_str());
			unloadedmap->lumpnamehash = quickncasehash(unloadedmap->lumpname, MAXMAPLUMPNAME);

			// Insert at the head, just because it's convenient.
			unloadedmap->next = unloadedmapheaders;
			unloadedmapheaders = unloadedmap;

			// Finally, copy into.
			unloadedmap->records = dummyrecord;
		}
	}

	gamedata->gotspraycans = 0;
	gamedata->numspraycans = js.spraycans.size();
	if (gamedata->spraycans)
	{
		Z_Free(gamedata->spraycans);
	}
	if (gamedata->numspraycans)
	{
		gamedata->spraycans = static_cast<candata_t*>(Z_Malloc(
			(gamedata->numspraycans * sizeof(candata_t)),
			PU_STATIC, NULL));

		for (size_t i = 0; i < gamedata->numspraycans; i++)
		{
			auto& can = js.spraycans[i];

			// Find the skin color index for the name
			bool foundcolor = false;
			for (size_t j = 0; j < numskincolors; j++)
			{
				if (can.color == skincolors[j].name)
				{
					gamedata->spraycans[i].col = j;
					foundcolor = true;
					break;
				}
			}
			if (!foundcolor)
			{
				// Invalid color name? Ignore the spraycan
				gamedata->numspraycans -= 1;
				i -= 1;
				continue;
			}

			gamedata->spraycans[i].map = NEXTMAP_INVALID;

			UINT16 mapnum = NEXTMAP_INVALID;
			if (!can.map.empty())
			{
				mapnum = G_MapNumber(can.map.c_str());
			}
			gamedata->spraycans[i].map = mapnum;
			if (mapnum >= nummapheaders)
			{
				// Can has not been grabbed on any map, this is intentional.
				continue;
			}

			if (gamedata->gotspraycans != i)
			{
				//CONS_Printf("LOAD - Swapping gotten can %u, color %s with prior ungotten can %u\n", i, skincolors[col].name, gamedata->gotspraycans);

				// All grabbed cans should be at the head of the list.
				// Let's swap with the can the disjoint occoured at.
				// This will prevent a gap from occouring on reload.
				candata_t copycan = gamedata->spraycans[gamedata->gotspraycans];
				gamedata->spraycans[gamedata->gotspraycans] = gamedata->spraycans[i];
				gamedata->spraycans[i] = copycan;

				mapheaderinfo[copycan.map]->cache_spraycan = i;
			}
			mapheaderinfo[mapnum]->cache_spraycan = gamedata->gotspraycans;
			gamedata->gotspraycans++;
		}
	}
	else
	{
		gamedata->spraycans = nullptr;
	}

	for (auto& cuppair : js.cups)
	{
		std::array<cupwindata_t, KARTGP_MAX> dummywindata {{}};
		cupheader_t* cup = nullptr;

		// Find the loaded cup
		for (cup = kartcupheaders; cup; cup = cup->next)
		{
			std::string cupname = std::string(cup->name);
			if (cupname == cuppair.first)
			{
				break;
			}
		}

		// Digest its data...
		for (size_t j = 0; j < std::min<size_t>(KARTGP_MAX, cuppair.second.records.size()); j++)
		{
			dummywindata[j].best_placement = cuppair.second.records[j].bestplacement;
			dummywindata[j].best_grade = static_cast<gp_rank_e>(cuppair.second.records[j].bestgrade);
			dummywindata[j].got_emerald = cuppair.second.records[j].gotemerald;

			dummywindata[j].best_skin.id = MAXSKINS;
			dummywindata[j].best_skin.unloaded = nullptr;

			bool skinfound = false;
			for (int skin = 0; skin < numskins; skin++)
			{
				std::string skinname = std::string(skins[skin].name);
				if (skinname == cuppair.second.records[j].bestskin)
				{
					skinreference_t ref {};
					ref.id = skin;
					ref.unloaded = nullptr;
					dummywindata[j].best_skin = ref;
					skinfound = true;
					break;
				}
			}
			if (skinfound)
			{
				continue;
			}
			for (auto unloadedskin = unloadedskins; unloadedskin; unloadedskin = unloadedskin->next)
			{
				std::string skinname = std::string(unloadedskin->name);
				if (skinname == cuppair.second.records[j].bestskin)
				{
					skinreference_t ref {};
					ref.id = MAXSKINS;
					ref.unloaded = unloadedskin;
					dummywindata[j].best_skin = ref;
					skinfound = true;
					break;
				}
			}
		}

		if (cup)
		{
			// We found a cup, so assign the windata.

			memcpy(cup->windata, dummywindata.data(), sizeof(cup->windata));
		}
		else if (dummywindata[0].best_placement != 0)
		{
			// Invalid, but we don't want to lose all the juicy statistics.
			// Instead, update a FILO linked list of "unloaded cupheaders".

			unloaded_cupheader_t *unloadedcup =
				static_cast<unloaded_cupheader_t*>(Z_Malloc(
					sizeof(unloaded_cupheader_t),
					PU_STATIC, NULL
				));

			// Establish properties, for later retrieval on file add.
			strlcpy(unloadedcup->name, cuppair.first.c_str(), sizeof(unloadedcup->name));
			unloadedcup->namehash = quickncasehash(unloadedcup->name, MAXCUPNAME);

			// Insert at the head, just because it's convenient.
			unloadedcup->next = unloadedcupheaders;
			unloadedcupheaders = unloadedcup;

			// Finally, copy into.
			memcpy(unloadedcup->windata, dummywindata.data(), sizeof(cup->windata));
		}
	}

	M_FinaliseGameData();
}

// G_LoadGameData
// Loads the main data file, which stores information such as emblems found, etc.
void G_LoadGameData(void)
{
	try
	{
		srb2::load_ng_gamedata();
	}
	catch (...)
	{
		CONS_Alert(CONS_ERROR, "NG Gamedata loading failed\n");
	}
}