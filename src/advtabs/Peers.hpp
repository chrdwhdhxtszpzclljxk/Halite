
#pragma once

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"

#include "../DdxEx.hpp"
#include "../Halite.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteDialogBase.hpp"
#include "../HaliteListManager.hpp"

class PeerListView :
	public CHaliteSortListViewCtrl<PeerListView, const hal::PeerDetail>,
	public CHaliteIni<PeerListView>,
	private boost::noncopyable
{
protected:
	typedef PeerListView thisClass;
	typedef CHaliteIni<thisClass> iniClass;
	typedef CHaliteSortListViewCtrl<thisClass, const hal::PeerDetail> listClass;
	typedef const hal::PeerDetail pD;

	friend class listClass;
	
	struct ColumnAdapters
	{
	
	typedef listClass::ColumnAdapter ColAdapter_t;
	
	struct SpeedDown : public ColAdapter_t
	{
		virtual bool less(pD& l, pD& r)	{ return l.speed.first < r.speed.first; }		
		virtual std::wstring print(pD& p) 
		{
			return (wformat(L"%1$.2fkb/s") % (p.speed.first/1024)).str(); 
		}		
	};
	
	struct SpeedUp : public ColAdapter_t
	{
		virtual bool less(pD& l, pD& r)	{ return l.speed.second < r.speed.second; }		
		virtual std::wstring print(pD& p) 
		{
			return (wformat(L"%1$.2fkb/s") % (p.speed.second/1024)).str(); 
		}		
	};
	
	};

public:	
	enum { 
		LISTVIEW_ID_MENU = IDR_LISTVIEW_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGPEER_LISTVIEW_ADV,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_DIALOGPEER_LISTVIEW_ADV_DEFAULTS
	};
	

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	thisClass() :
		iniClass("listviews/advPeers", "PeerListView"),
		listClass(true,false,false)
	{					
		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

		// "Peer;Country;Download;Upload;Type;Client,Status"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 7> widths = {100,20,70,70,70,100,200};
		array<int, 7> order = {0,1,2,3,4,5,6};
		array<bool, 7> visible = {true,true,true,true,true,true,true};
		
		SetDefaults(names, widths, order, visible, true);
		Load();
	}
	
	void saveSettings()
	{
		GetListViewDetails();
		save();
	}
	
	void OnAttach()
	{
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
		SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		ApplyDetails();
		
		SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedDown());
		SetColumnSortType(3, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedUp());
	}
	
	void OnDestroy()
	{
		saveSettings();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("listview", 
			boost::serialization::base_object<listClass>(*this));
	}
	
	pD CustomItemConversion(LVCompareParam* param, int iSortCol)
	{			
		return peerDetails_[param->dwItemData];
	}		
	
	int CustomItemComparision(pD left, pD right, int iSortCol)
	{
		ColumnAdapter* pCA = getColumnAdapter(iSortCol);
		
		if (pCA)
			return (pCA->less(left, right)) ? 1 : -1;
		else 
			return 0;
	}
	
	void uiUpdate(const hal::TorrentDetails& tD);
	
private:
	hal::PeerDetails peerDetails_;
};

class AdvPeerDialog :
	public CHalTabPageImpl<AdvPeerDialog>,
	public CHaliteDialogBase<AdvPeerDialog>,
	public CDialogResize<AdvPeerDialog>
{
protected:
	typedef AdvPeerDialog thisClass;
	typedef CHalTabPageImpl<thisClass> baseClass;
	typedef CDialogResize<thisClass> resizeClass;
	typedef CHaliteDialogBase<AdvPeerDialog> dialogBaseClass;

public:
	enum { IDD = IDD_ADVPEER };

	AdvPeerDialog(HaliteWindow& halWindow) :
		dialogBaseClass(halWindow)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_PEERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);
	
	void uiUpdate(const hal::TorrentDetails& tD);

protected:
	PeerListView peerList_;
};
