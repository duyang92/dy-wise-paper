#include<fstream>
#include<unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include<unordered_set>
#include <dirent.h>
using namespace std;
#define IP_ACTUAL_SIM "ip_actual_sim.txt";
#define TEXT_ACTUAL_SIM "text_dataset/text_actual_sim_";
#define RECOMMENDATION_ACTUAL_SIM "recommendation_dataset/recommendation_actual_sim_";
#define IP_DATASET_FOLDER "ip_dataset"
#define TEXT_DATASET "text_dataset/processed_text_dataset.txt"
#define RECOMMENDATION_DATASET "recommendation_dataset/processed_recommendation_dataset.txt"
#ifndef QUERY_SIZE
#define QUERY_SIZE 100
#endif
template<typename T>
class DataReader {
    public:
    T* t;
    int size;
    unordered_map<long long,double> actual_sim;
    unordered_map<long long,double> estimated_sim;
    DataReader(T* t,int s)
    {
        this->t=t;
        this->size=s;
    }
    void clear()
    {
        t->clear();
        actual_sim.clear();
        estimated_sim.clear();
    }
    inline double now_us ()
    {
        struct timespec tv;
        clock_gettime(CLOCK_MONOTONIC, &tv);
        return (tv.tv_sec * (uint64_t) 1000000 + (double)tv.tv_nsec/1000);
    }
    void load_actual_sim(string type){
        string file_name;
        if(type=="ip")
        {
            file_name=IP_ACTUAL_SIM;
            file_name+=to_string(this->size)+".txt";
        }
        else if(type=="text")
        {
            file_name=TEXT_ACTUAL_SIM;
            file_name+=to_string(this->size)+".txt";
        }
        else if(type=="rec")
        {
            file_name=RECOMMENDATION_ACTUAL_SIM;
            file_name+=to_string(this->size)+".txt";
        }
        ifstream file(file_name);
        long long code;
        double val;
        while(file>>code&&file>>val)
        {
            actual_sim[code]=val;
            // cout<<code<<" "<<val<<endl;
        }
        cout<<"dataset loaded"<<endl;
    }
    void test_ip(int limit)
    {
        string path=IP_DATASET_FOLDER;
        int id=0;
        DIR* dir = opendir(path.c_str());
        if (dir == nullptr) {
            perror("opendir");
        }
        double total_time=0;
        struct dirent* entry;
        long long total_word=0;
        while ((entry = readdir(dir)) != nullptr) {
            // Skip special directories "." and ".."
            if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
                std::string file_name = path + "/" + entry->d_name;
                ifstream file(file_name);
                string src;string dst;
                while (file>>src&&file>>dst)
                {
                    double start=now_us();
                    t->insert(id,src);
                    t->insert(id,dst);
                    double end=now_us();
                    total_time+=end-start;
                    total_word+=2;
                }
                #ifdef KEYWORD_SKETCH
                t->bf->clear();
                #endif
                #if defined(ICWS_SKETCH) || defined(FAST_ICWS_SKETCH) || defined(BAGMINHASH_SKETCH) || defined(DARTMINHASH_SKETCH)
                double start=now_us();
                t->flush();
                double end=now_us();
                total_time+=end-start;
                #endif
                id++;
                cout<<id<<endl;
                if(limit==id) break;
            }
        }
        cout<<"Throughput for IP dataset: "<<total_word/total_time<<" Mops"<<endl;
        closedir(dir);  
        double are=0;
        double pairs=0;
        double query_time=0;
        for(int i=0;i<QUERY_SIZE;i++)
        {
            for(int j=i+1;j<QUERY_SIZE;j++)
            {
                double start=now_us();
                double est_sim=t->estimate(i,j);
                double end=now_us();
                query_time+=end-start;
                double actual=actual_sim[i*(limit+1)+j];
                if(actual==0)
                {
                    continue;
                }
                are+=abs(est_sim-actual)/actual;   
            }
        }
        cout<<"Average Relative Error for IP dataset: "<<are/(QUERY_SIZE*(QUERY_SIZE-1)/2)<<endl;
        cout<<"Average Query Time for IP dataset: "<<query_time/(QUERY_SIZE*(QUERY_SIZE-1)/2)<<" us"<<endl;
    }
    void test_quora(double threshold)
    {
        int limit=this->size;
        string path="quora_questions.txt";
        ifstream file(path);
        string line;
        int id=0;
        double total_time=0;
        long long total_word=0;
        while(getline(file,line))
        {
            string word;
            std::stringstream stream(line);
            while(getline(stream,word,' '))
            {
                double start=now_us();
                t->insert(id,word);
                total_word++;
                double end=now_us();
                total_time+=end-start;
            }
            id++;
            if(limit==id) break;
        }
        cout<<"Throughput for quora dataset: "<<total_word/(total_time)<<" Mops"<<endl;
        double recall=0;
        double precision=0;
        double f1=0;
        long long pairs=0;
        ifstream dup_list("quora_dup_list.txt");
        int true_pos=0;
        int false_pos=0;
        int false_neg=0;
        int qid1, qid2;
        unordered_map<long long,int> dups;
        while(dup_list>>qid1>>qid2)
        {
            dups[(long long)qid1*1000000+qid2]=1;
        }
        double query_time=0;
        for(int i=0;i<QUERY_SIZE;i++)
        {
            for(int j=i+1;j<QUERY_SIZE;j++)
            {
                double start=now_us();
                double est_sim=t->estimate(i,j);
                double end=now_us();
                query_time+=end-start;
                if(est_sim>=threshold)
                {
                    if(dups.find((long long)i*1000000+j)!=dups.end())
                    {
                        true_pos++;
                    }
                    else
                    {
                        false_pos++;
                    }
                }
                else
                {
                    if(dups.find((long long)i*1000000+j)!=dups.end())
                    {
                        false_neg++;
                    }
                }
                pairs++;
            }
        }
        cout<<"True Positives: "<<true_pos<<endl;
        cout<<"False Positives: "<<false_pos<<endl;
        cout<<"False Negatives: "<<false_neg<<endl;
        recall=(double)true_pos/(true_pos+false_neg);
        precision=(double)true_pos/(true_pos+false_pos);
        f1=2*recall*precision/(recall+precision);
        cout<<"Recall: "<<recall<<endl;
        cout<<"Precision: "<<precision<<endl;
        cout<<"F1 Score: "<<f1<<endl;
        cout<<"Average Query Time for quora dataset: "<<query_time/pairs<<" us"<<endl;
        cout<<"memory used: "<<t->usedmem()<<" bytes"<<endl;
    }
    void test_CORE(double threshold)
    {
        int limit=this->size;
        string path="core_dataset";
        ifstream file(path);
        string line;
        int number=0;
        double total_time=0;
        long long total_word=0;
        vector<int> all_ids; 
        while(getline(file,line))
        {
            string word;
            string id_str;
            std::stringstream stream(line);
            getline(stream,id_str,' ');
            int id= stoi(id_str);
            all_ids.push_back(id);
            while(getline(stream,word,' '))
            {
                double start=now_us();
                t->insert(id,word);
                total_word++;
                double end=now_us();
                total_time+=end-start;
            }
            number++;
            if(limit==number) break;
        }
        cout<<"Throughput for CORE dataset: "<<total_word/(total_time)<<" Mops"<<endl;
        double recall=0;
        double precision=0;
        double f1=0;
        long long pairs=0;
        ifstream dup_list("core_id_list");
        int true_pos=0;
        int false_pos=0;
        int false_neg=0;
        int qid1, qid2;
        unordered_map<string,int> dups;
        while(getline(dup_list,line))
        {
            string id_str;
            string duplicates;
            std::stringstream stream(line);
            getline(stream,id_str,' ');
            while(getline(stream,duplicates,' '))
            {
                dups[id_str+"-"+duplicates]=1;
            }
        }
        double query_time=0;
        for(int i=0;i<QUERY_SIZE;i++)
        {
            for(int j=i+1;j<QUERY_SIZE;j++)
            {
                double start=now_us();
                double est_sim=t->estimate(all_ids[i],all_ids[j]);
                double end=now_us();
                query_time+=end-start;
                if(est_sim>=threshold)
                {
                    if(dups.find(to_string(all_ids[i])+"-"+to_string(all_ids[j]))!=dups.end())
                    {
                        true_pos++;
                    }
                    else
                    {
                        false_pos++;
                    }
                }
                else
                {
                    if(dups.find(to_string(all_ids[i])+"-"+to_string(all_ids[j]))!=dups.end())
                    {
                        false_neg++;
                    }
                }
                pairs++;
            }
        }
        cout<<"True Positives: "<<true_pos<<endl;
        cout<<"False Positives: "<<false_pos<<endl;
        cout<<"False Negatives: "<<false_neg<<endl;
        recall=(double)true_pos/(true_pos+false_neg);
        precision=(double)true_pos/(true_pos+false_pos);
        f1=2*recall*precision/(recall+precision);
        cout<<"Recall: "<<recall<<endl;
        cout<<"Precision: "<<precision<<endl;
        cout<<"F1 Score: "<<f1<<endl;
        cout<<"Running time"<<query_time<<" us"<<endl;
        cout<<"memory used: "<<t->usedmem()<<" bytes"<<endl;
    }
    void test_cddb(double threshold)
    {
        int limit=this->size;
        string path="cddb_dataset";
        ifstream file(path);
        string line;
        int number=0;
        double total_time=0;
        long long total_word=0;
        vector<int> all_ids; 
        while(getline(file,line))
        {
            string word;
            string id_str;
            std::stringstream stream(line);
            getline(stream,id_str,' ');
            int id= stoi(id_str);
            all_ids.push_back(id);
            while(getline(stream,word,' '))
            {
                double start=now_us();
                t->insert(id,word);
                total_word++;
                double end=now_us();
                total_time+=end-start;
            }
            number++;
            if(limit==number) break;
        }
        cout<<"Throughput for CDDB dataset: "<<total_word/(total_time)<<" Mops"<<endl;
        double recall=0;
        double precision=0;
        double f1=0;
        long long pairs=0;
        ifstream dup_list("cddb_id_list");
        int true_pos=0;
        int false_pos=0;
        int false_neg=0;
        int qid1, qid2;
        unordered_map<string,int> dups;
        while(getline(dup_list,line))
        {
            string id_str;
            string duplicates;
            std::stringstream stream(line);
            getline(stream,id_str,' ');
            while(getline(stream,duplicates,' '))
            {
                dups[id_str+"-"+duplicates]=1;
            }
        }
        double query_time=0;
        for(int i=0;i<QUERY_SIZE;i++)
        {
            for(int j=i+1;j<QUERY_SIZE;j++)
            {
                double start=now_us();
                double est_sim=t->estimate(all_ids[i],all_ids[j]);
                double end=now_us();
                query_time+=end-start;
                if(est_sim>=threshold)
                {
                    if(dups.find(to_string(all_ids[i])+"-"+to_string(all_ids[j]))!=dups.end())
                    {
                        true_pos++;
                    }
                    else
                    {
                        false_pos++;
                    }
                }
                else
                {
                    if(dups.find(to_string(all_ids[i])+"-"+to_string(all_ids[j]))!=dups.end())
                    {
                        false_neg++;
                    }
                }
                pairs++;
            }
        }
        cout<<"True Positives: "<<true_pos<<endl;
        cout<<"False Positives: "<<false_pos<<endl;
        cout<<"False Negatives: "<<false_neg<<endl;
        recall=(double)true_pos/(true_pos+false_neg);
        precision=(double)true_pos/(true_pos+false_pos);
        f1=2*recall*precision/(recall+precision);
        cout<<"Recall: "<<recall<<endl;
        cout<<"Precision: "<<precision<<endl;
        cout<<"F1 Score: "<<f1<<endl;
        cout<<"Running time"<<query_time<<" us"<<endl;
    }
    void test_restaurant(double threshold)
    {
        int limit=this->size;
        string path="restaurant_dataset";
        ifstream file(path);
        string line;
        int number=0;
        double total_time=0;
        long long total_word=0;
        vector<int> all_ids; 
        while(getline(file,line))
        {
            string word;
            string id_str;
            std::stringstream stream(line);
            getline(stream,id_str,' ');
            int id= stoi(id_str);
            all_ids.push_back(id);
            while(getline(stream,word,' '))
            {
                double start=now_us();
                t->insert(id,word);
                total_word++;
                double end=now_us();
                total_time+=end-start;
            }
            number++;
            if(limit==number) break;
        }
        cout<<"Throughput for restaurant dataset: "<<total_word/(total_time)<<" Mops"<<endl;
        double recall=0;
        double precision=0;
        double f1=0;
        long long pairs=0;
        ifstream dup_list("restaurant_id_list");
        int true_pos=0;
        int false_pos=0;
        int false_neg=0;
        int qid1, qid2;
        unordered_map<string,int> dups;
        while(getline(dup_list,line))
        {
            string id_str;
            string duplicates;
            std::stringstream stream(line);
            getline(stream,id_str,' ');
            while(getline(stream,duplicates,' '))
            {
                dups[id_str+"-"+duplicates]=1;
            }
        }
        int n=all_ids.size();
        n=min(n,QUERY_SIZE);
        double query_time=0;
        for(int i=0;i<n;i++)
        {
            for(int j=i+1;j<n;j++)
            {
                double start=now_us();
                double est_sim=t->estimate(all_ids[i],all_ids[j]);
                double end=now_us();
                query_time+=end-start;
                if(est_sim>=threshold)
                {
                    if(dups.find(to_string(all_ids[i])+"-"+to_string(all_ids[j]))!=dups.end())
                    {
                        true_pos++;
                    }
                    else
                    {
                        false_pos++;
                    }
                }
                else
                {
                    if(dups.find(to_string(all_ids[i])+"-"+to_string(all_ids[j]))!=dups.end())
                    {
                        false_neg++;
                    }
                }
                pairs++;
            }
        }
        cout<<"True Positives: "<<true_pos<<endl;
        cout<<"False Positives: "<<false_pos<<endl;
        cout<<"False Negatives: "<<false_neg<<endl;
        recall=(double)true_pos/(true_pos+false_neg);
        precision=(double)true_pos/(true_pos+false_pos);
        f1=2*recall*precision/(recall+precision);
        cout<<"Recall: "<<recall<<endl;
        cout<<"Precision: "<<precision<<endl;
        cout<<"F1 Score: "<<f1<<endl;
        cout<<"Running time"<<query_time<<" us"<<endl;
    }
    void test_kuairec(double threshold)
    {
        int limit=this->size;
        string path="kuai_rec/user_video_train.txt";
        ifstream file(path);
        ifstream ground_truth("kuai_rec/user_video_test.txt");
        unordered_map<uint64_t,unordered_set<uint64_t>> ground_truth_map;
        string line_gt;
        while(getline(ground_truth,line_gt))
        {
            string user_id_str;
            string video_id_str;
            std::stringstream stream(line_gt);
            getline(stream,user_id_str,' ');
            while (getline(stream,video_id_str,' '))
            {
                uint64_t user_id=stoull(user_id_str);
                uint64_t video_id=stoull(video_id_str);
                ground_truth_map[user_id].insert(video_id);
            }
        }
        string line;
        int number=0;
        double total_time=0;
        long long total_word=0;
        vector<int> all_ids; 
        while(getline(file,line))
        {
            string word;
            string id_str;
            std::stringstream stream(line);
            getline(stream,id_str,' ');
            int id= stoi(id_str);
            all_ids.push_back(id);
            while(getline(stream,word,' '))
            {
                double start=now_us();
                t->insert(id,word);
                total_word++;
                double end=now_us();
                total_time+=end-start;
            }
            number++;
            if(limit==number) break;
        }
        cout<<"Throughput for kuairec dataset: "<<total_word/(total_time)<<" Mops"<<endl;
        double recall=0;
        double precision=0;
        double f1=0;
        long long pairs=0;
        int true_pos=0;
        int false_pos=0;
        int false_neg=0;
        int hit50=0;
        int hit100=0;
        int n=all_ids.size();
        n=min(n,QUERY_SIZE);
        double query_time=0;
        
        for(int i=0;i<n;i++)
        {
            unordered_set<int> recommend_videos;
            for(int j=0;j<n;j++)
            {
                if(i==j) continue;
                double start=now_us();
                double est_sim=t->estimate(all_ids[i],all_ids[j]);
                double end=now_us();
                query_time+=end-start;
                if(est_sim>=threshold)
                {
                    for(auto v: ground_truth_map[all_ids[j]])
                    {
                        recommend_videos.insert(v);
                    }
                }
            }
            for(auto v: ground_truth_map[all_ids[i]])
            {
                if(recommend_videos.find(v)!=recommend_videos.end())
                {
                    true_pos++;
                }
                else
                {
                    false_neg++;
                }
            }
            int hit=0;
            int idx=0;
            for(auto v: recommend_videos)
            {
                if(ground_truth_map[all_ids[i]].find(v)==ground_truth_map[all_ids[i]].end())
                {
                    false_pos++;
                }
                else
                {
                    hit++;
                }
                idx++;
                if(idx==50) hit50+=hit;
                if(idx==100) hit100+=hit;
            }
        }
        cout<<"True Positives: "<<true_pos<<endl;
        cout<<"False Positives: "<<false_pos<<endl;
        cout<<"False Negatives: "<<false_neg<<endl;
        recall=(double)true_pos/(true_pos+false_neg);
        precision=(double)true_pos/(true_pos+false_pos);
        f1=2*recall*precision/(recall+precision);
        cout<<"Recall: "<<recall<<endl;
        cout<<"Precision: "<<precision<<endl;
        cout<<"F1 Score: "<<f1<<endl;
        cout<<"Hit@50: "<<(double)hit50/(n*50)<<endl;
        cout<<"Hit@100: "<<(double)hit100/(n*100)<<endl;
        cout<<"Running time"<<total_time+query_time<<" us"<<endl;
    }
    void test_text_or_recommendation(string type)
    {
        int limit=this->size;
        string path;
        if(type=="text")
        {
            path=TEXT_DATASET;
        }
        else if(type=="rec")
        {
            path=RECOMMENDATION_DATASET;
        }
        ifstream file(path);
        int id=1;
        string line;
        double total_time=0;
        long long total_word=0;
        while(getline(file,line))
        {
            string word;
            std::stringstream stream(line);
            while(getline(stream,word,' '))
            {
//                 cout<<word<<endl;
                double start=now_us();
                t->insert(id,word);
                total_word++;
                double end=now_us();
                total_time+=end-start;
            }
            if(limit==id) break;
            id++;
        }
//         t->usedmem();s
        cout<<"Throughput for "<<type<<" dataset: "<<total_word/(total_time)<<" Mops"<<endl;
        double are=0;
        double mae=0;
        long long pairs=0;
        double query_time=0;
        for(int i=1;i<=QUERY_SIZE;i++)
        {
            for(int j=i+1;j<=QUERY_SIZE;j++)
            {
//                 cout<<i<<" "<<j<<endl;
//                 if(i==1&&j==10) continue;
                double start=now_us();
                double est_sim=t->estimate(i,j);
                double end=now_us();
//                 t->usedmem();
                query_time+=end-start;
                double actual=actual_sim[(long long)i*(100001)+j];
                if(actual==0)
                {
                    continue;
                }
                mae+=abs(est_sim-actual);
                are+=abs(est_sim-actual)/actual;
                pairs++;
            }
        }
        cout<<pairs<<endl;
        cout<<"Average Absolute Error for "<<type<<" dataset: "<<mae/pairs<<endl;
        cout<<"Average Relative Error for "<<type<<" dataset: "<<are/pairs<<endl;
        cout<<"Average Query Time for "<<type<<" dataset: "<<query_time/pairs<<" us"<<endl;
        cout<<"memory used: "<<t->usedmem()<<" bytes"<<endl;
        ofstream outfile("result.txt",ios::app);
        #ifdef KEYWORD_SKETCH
        outfile<<"our"<<endl;
        #endif
        #ifdef ICWS_SKETCH
        outfile<<"ICWS"<<endl;
        #endif
        #ifdef FAST_ICWS_SKETCH
        outfile<<"FICWS"<<endl;
        #endif
        #ifdef BAGMINHASH_SKETCH
        outfile<<"BagMH"<<endl;
        #endif
        #ifdef DARTMINHASH_SKETCH
        #ifdef HTWE
        outfile<<"DartMH"<<endl;
        #endif
        #ifdef SWE
        outfile<<"DartMH+SWE"<<endl;
        #endif
        #endif
        #ifdef DSS2
        #ifdef LAZY
        outfile<<"DSSE+LU"<<endl;
        #else
        outfile<<"DSSE"<<endl;
        #endif
        #endif
        outfile<<type<<endl;
        outfile<<are/pairs<<endl;
        outfile<<mae/pairs<<endl;
        outfile<<query_time/pairs<<endl;
        outfile<<total_word/(total_time)<<endl;
        outfile<<t->usedmem()/1024<<endl;
        outfile.close();
    }
};
