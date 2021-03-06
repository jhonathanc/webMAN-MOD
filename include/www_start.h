if(conn_s_p == START_DAEMON || conn_s_p == REFRESH_CONTENT)
{
	bool do_sleep = true;

	#ifdef WM_PROXY_SPRX
	apply_remaps();
	#endif

	if(conn_s_p == START_DAEMON)
	{
		if(file_exists("/dev_hdd0/ps3-updatelist.txt") || !payload_ps3hen)
			vshnet_setUpdateUrl("http://127.0.0.1/dev_hdd0/ps3-updatelist.txt"); // custom update file

		#ifndef ENGLISH_ONLY
		update_language();
		#endif
		make_fb_xml();

		if(profile || !(webman_config->wmstart))
		{
			char cfw_info[20];
			get_cobra_version(cfw_info);

			if(payload_ps3hen)
			{
				sprintf(param,	"%s\n"
								"%s %s", WM_APP_VERSION, cfw_info + 4, STR_ENABLED);
			}
			else
			{
				sys_ppu_thread_sleep(9);
				sprintf(param,	"%s\n"
								"%s %s" EDITION, WM_APP_VERSION, fw_version, cfw_info);
			}
			show_msg(param); do_sleep = false;
		}

		if(webman_config->bootd) wait_for("/dev_usb", webman_config->bootd); // wait for any usb

		// is JAP?
		int enter_button = 1;
		xsetting_0AF1F161()->GetEnterButtonAssign(&enter_button);
		CELL_PAD_CIRCLE_BTN = enter_button ? CELL_PAD_CTRL_CIRCLE : CELL_PAD_CTRL_CROSS;
	}
	else //if(conn_s_p == REFRESH_CONTENT)
	{
		{DELETE_CACHED_GAMES} // refresh XML will force "refresh HTML" to rebuild the cache file
	}

	mkdirs(param); // make hdd0 dirs GAMES, PS3ISO, PS2ISO, packages, etc.


	//////////// usb ports ////////////
	for(u8 indx = 5, d = 6; d < 128; d++)
	{
		sprintf(param, "/dev_usb%03i", d);
		if(isDir(param)) {strcpy(drives[indx++], param); if(indx > 6) break;}
	}
	///////////////////////////////////


	check_cover_folders(param);

	// Use system icons if wm_icons don't exist
	for(u8 i = 0; i < 14; i++)
	{
		if(not_exists(wm_icons[i]))
		{
			sprintf(param, VSH_RESOURCE_DIR "explore/icon/%s", wm_icons[i] + 23); strcpy(wm_icons[i], param);
			if(file_exists(param)) continue;

			char *icon = wm_icons[i] + 32;
			if(i == gPS3 || i == iPS3)	sprintf(icon, "user/024.png"); else // ps3
			if(i == gPSX || i == iPSX)	sprintf(icon, "user/026.png"); else // psx
			if(i == gPS2 || i == iPS2)	sprintf(icon, "user/025.png"); else // ps2
			if(i == gPSP || i == iPSP)	sprintf(icon, "user/022.png"); else // psp
			if(i == gDVD || i == iDVD)	sprintf(icon, "user/023.png"); else // dvd
			if(i == iROM || i == iBDVD)	strcpy(wm_icons[i], wm_icons[iPS3]); else
										sprintf(icon + 5, "icon_home.png"); // setup / eject
		}
	}

	#ifndef EMBED_JS
	css_exists = file_exists(COMMON_CSS);
	common_js_exists = file_exists(COMMON_SCRIPT_JS);
	#endif

	#ifdef NOSINGSTAR
	no_singstar_icon();
	#endif

	#ifndef LITE_EDITION
	chart_init = 0;
	#endif

	sys_ppu_thread_t t_id;
	sys_ppu_thread_create(&t_id, update_xml_thread, conn_s_p, THREAD_PRIO, THREAD_STACK_SIZE_UPDATE_XML, SYS_PPU_THREAD_CREATE_NORMAL, THREAD_NAME_CMD);

	if(conn_s_p == START_DAEMON)
	{
		#ifdef COBRA_ONLY
		cobra_read_config(cobra_config);

		// cobra spoofer not working since 4.53
		if(webman_config->nospoof || (c_firmware >= 4.53f))
		{
			cobra_config->spoof_version  = 0;
			cobra_config->spoof_revision = 0;
		}
		else
		{
			cobra_config->spoof_version = 0x0486;
			cobra_config->spoof_revision = 67896; //0x00010938 // 4.85 = 67869; // 0x0001091d
		}

		if( cobra_config->ps2softemu == 0 && cobra_get_ps2_emu_type() == PS2_EMU_SW )
			cobra_config->ps2softemu =  1;

		cobra_write_config(cobra_config);
		#endif

		#ifdef SPOOF_CONSOLEID
		spoof_idps_psid();
		#endif

		if(!payload_ps3hen) { ENABLE_INGAME_SCREENSHOT }

		#ifdef COBRA_ONLY
		#ifdef REMOVE_SYSCALLS
		if(webman_config->spp & 1) //remove syscalls & history
		{
			if(!payload_ps3hen) sys_ppu_thread_sleep(5); do_sleep = false;

			remove_cfw_syscalls(webman_config->keep_ccapi);
			delete_history(true);
		}
		else
		#endif
		if(webman_config->spp & 2) //remove history & block psn servers (offline mode)
		{
			delete_history(false);
			block_online_servers(false);
		}
		#ifdef OFFLINE_INGAME
		if(file_exists(WMNET_DISABLED)) //re-enable network (force offline in game)
		{
			net_status = 1;
			poll_start_play_time();
		}
		#endif
		#endif //#ifdef COBRA_ONLY

		if(do_sleep) sys_ppu_thread_sleep(1);

		#ifdef COBRA_ONLY
		{sys_map_path((char*)"/dev_flash/vsh/resource/coldboot_stereo.ac3", NULL);}
		{sys_map_path((char*)"/dev_flash/vsh/resource/coldboot_multi.ac3",  NULL);}
		map_earth(0, param);
		#endif
	}

	sys_ppu_thread_exit(0);
}
