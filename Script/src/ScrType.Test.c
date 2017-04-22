/*  Fondement Michtam
 *  Copyright (C) 2011 Xavier Lacroix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <StackEnv.h>
#include <Classes.h>
#include <ScrModule.h>

extern ScrScopeBrowser *ScrScopeBrowserDisplay(void);
extern ScrDataBrowser *ScrDataDisplay(void);

main() {
	ScrLinkScope *Modules;
	ScrDataVal *MyModule;
	ScrModuleDesc *ModDesc;
	int ModNum;
	ScrScopeBrowser *Output;
	ScrModuleParser *Files;
	ScrDataBrowser *DataBrowse;
	EnvOpen(4096,4096);
	char *Path[] = {".","..",0};
	char *Dependencies[] = {0};
	Modules = ScrLinkScopeNew(ScrVocabularyNew());
	Files = ScrSetParserPath(Path,(0!=0));
	ModDesc = Call(Files,Source,1("Type.Test.lxs"));
	ModNum = Call(Modules,InsertModule,3("MyModule",ModDesc,Dependencies));
	MyModule = Call(Modules,GetModule,1(ModNum));
	Output = ScrScopeBrowserDisplay();
	Call(Modules,Browse,2(ModNum,Output));
	DataBrowse = ScrDataDisplay();
	Call(MyModule,Browse,1(DataBrowse));
	Call(Modules,ReleaseModule,1(ModNum));
	EnvClose();
}
