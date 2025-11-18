/***********************************************************************************************
created: 		2025-05-02

author:			chensong

purpose:		  ts 的 切片管理  释放，


输赢不重要，答案对你们有什么意义才重要。

光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/
#include "libmedia_transfer_protocol/libhls/cfragment_window.h"
#include <sstream> 
#include <cmath> 


#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"



namespace libmedia_transfer_protocol {
	namespace libhls
	{
		namespace
		{
			static FragmentPtr    fragment_null;
		}

		FragmentWindow::FragmentWindow(int32_t size )
			:window_size_(size)
		{ 
		}
		FragmentWindow::~FragmentWindow()
		{}
 
		//增加一个切片
		void FragmentWindow::AppendFragment(FragmentPtr &&fragment)
		{
			{
				std::lock_guard<std::mutex> lk(lock_);
				fragments_.emplace_back(std::move(fragment));
			}

			Shrink();

			UpdatePlayList();
		}
		

		// 获取一个空闲切片  没有空闲切片就创建一个切片
		FragmentPtr FragmentWindow::GetIdleFragment()
		{
			std::lock_guard<std::mutex> lk(lock_);

			if (free_fragments_.empty())
			{
				return  std::make_shared<Fragment>();
			}
			 auto p =  free_fragments_[0];
			 free_fragments_.erase(free_fragments_.begin() );
			 return p;
		}

		const FragmentPtr &FragmentWindow::GetFragmentByName(const std::string & name)  
		{
			std::lock_guard<std::mutex> lk(lock_);
			for (auto & f : fragments_)
			{
				if (f->FileName() == name)
				{
					return f;
				}
			}
			return fragment_null;
		}
		//  
		std::string FragmentWindow::GetPlayList()  
		{
			std::lock_guard<std::mutex> lk(lock_);
			return playlist_;
		}

		// 更新m3u8
		void FragmentWindow::UpdatePlayList()
		{
			std::lock_guard<std::mutex> lk(lock_);
			int32_t  remove_index = -1;
			if (fragments_.size() <= window_size_)
			{
				return;
			}
			remove_index = fragments_.size() - window_size_;

			for (int32_t i = 0; i < remove_index && !fragments_.empty(); i++)
			{
				auto p = *fragments_.begin();
				fragments_.erase(fragments_.begin());
				p->Reset();
				free_fragments_.emplace_back(std::move(p));
			}
		}


		//释放老的切片数据 过期啦~~~
		void FragmentWindow::Shrink()
		{
			std::lock_guard<std::mutex> lk(lock_);
			if (fragments_.empty() || fragments_.size() < 3)
			{
				return;
			}
	
			/*
			#EXTM3U
			#EXT-X-VERSION:3
			#EXT-X-TARGETDURATION:10
			#EXT-X-MEDIA-SEQUENCE:1057
			#EXT_X_TOTAL_DURATION:1800
			#EXTINF:10.00,
			34020000001320000001_34020000001320000002-20250329003001-1057.ts
			#EXTINF:10.00,
			34020000001320000001_34020000001320000002-20250329003011-1058.ts
			#EXT-X-ENDLIST

			*/
			std::ostringstream ss;

			ss << "#EXTM3U\n";
			//版本
			ss << "#EXT-X-VERSION:3 \n";
			// 最大切片时长
			int32_t i = fragments_.size() > 5 ? (fragments_.size() - 5) : 0;
			int32_t j = i;
			int32_t  max_duration = 0;
			for (; j < fragments_.size(); ++j)
			{
				max_duration = std::max(max_duration, (int32_t )fragments_[j]->Duration());
			}
			int32_t  target_duration = (int32_t) ceil((max_duration / 1000.0));

			ss << "#EXT-X-TARGETDURATION:" << target_duration << "\n";
			//切片序号
			ss << "#EXT-X-MEDIA-SEQUENCE:" << fragments_[i]->SequenceNo() << "\n";
			//ss << "#EXT_X_TOTAL_DURATION: \n";
			ss.precision(3);
			ss.setf(std::ios::fixed, std::ios::floatfield);
			for (; i < fragments_.size(); ++i)
			{
				//切片时长
				ss << "#EXTINF:" << fragments_[i]->Duration()/1000.0 <<"\n";
				ss << fragments_[i]->FileName() << "\n";
#if TEST_TS
				TestStreamWriter test_w(fragments_[i]->FileName());
				test_w.Write(fragments_[i]->FragmentData()->Data(), fragments_[i]->FragmentData()->PacketSize());
#endif // 
			}
			ss << "#EXT-X-ENDLIST\n";


			playlist_.clear();
			playlist_ = std::move(ss.str());

			//LIBHLS_LOG(LS_INFO) <<""

			LIBHLS_LOG(LS_INFO) << "playlist:" << playlist_;
		}
	}
}