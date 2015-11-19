#include "parameter_loader.h"
namespace multiverso
{
    namespace wordembedding
    {
        ParameterLoader::ParameterLoader(Option *option,
            WordEmbedding *WordEmbedding)
        {
            option_ = option;
            WordEmbedding_ = WordEmbedding;

            parse_and_request_count_ = 0;

            //the log which will store the begin and end time of ParseAndRequest
            char log_name[100];
            sprintf(log_name, "parameter_loader%s.txt", g_log_suffix.c_str());
            log_file_ = fopen(log_name, "w");
        }
	
        void ParameterLoader::ParseAndRequest(
            multiverso::DataBlockBase *data_block)
        {
            if (parse_and_request_count_ == 0)
            {
                start_ = clock();
            }

            fprintf(log_file_, "%lf\n", (clock()) / (double)CLOCKS_PER_SEC);
            multiverso::Log::Info("Rank %d [Parameterloader]------begin %d\n",
                multiverso::Multiverso::ProcessRank(), parse_and_request_count_);
            ++parse_and_request_count_;

            DataBlock *data = reinterpret_cast<DataBlock*>(data_block);
            //Step 1, compute the parameters which will be used when the trainers begin 
            multiverso::Log::Info("Rank %d [Parameterloader]------[PrepareParameter]------begin %d\n", multiverso::Multiverso::ProcessRank()
				,parse_and_request_count_);
			
            WordEmbedding_->PrepareParameter(data);
			multiverso::Log::Info("Rank %d [Parameterloader]------[PrepareParameter]------end %d\n", multiverso::Multiverso::ProcessRank(), parse_and_request_count_);
            //Step 2, Request the parameter
			multiverso::Log::Info("Rank %d [Parameterloader]------[RequestParameter]------begin %d\n", multiverso::Multiverso::ProcessRank(), parse_and_request_count_);
            RequestParameter(data);
			multiverso::Log::Info("Rank %d [Parameterloader]------[RequestParameter]------end %d\n", multiverso::Multiverso::ProcessRank(), parse_and_request_count_);
            //Step 3, store the needed parameters in data_block
            
			multiverso::Log::Info("Rank %d [Parameterloader]------end %d\n",
                multiverso::Multiverso::ProcessRank(), parse_and_request_count_ - 1);
            fprintf(log_file_, "%lf\n", (clock()) / (double)CLOCKS_PER_SEC);
            fflush(log_file_);
        }

        void ParameterLoader::RequestParameter(DataBlock *data_block)
		{
            //If the data_block is the last one, we need to dump 
            //the input-embedding weights
            if (data_block->Type() == DataBlockType::Test)
                RequestTable(kInputEmbeddingTableId);

            RequestRow(kWordCountActualTableId, 0);
            for (auto node : data_block->input_nodes)
                RequestRow(kInputEmbeddingTableId, node);
            for (auto node : data_block->output_nodes)
                RequestRow(kEmbeddingOutputTableId, node);
		
			if (option_->use_adagrad)
            {
                for (auto node : data_block->input_nodes)
                    RequestRow(kSumGradient2IETableId, node);
                for (auto node : data_block->output_nodes)
                    RequestRow(kSumGradient2EOTableId, node);
			}
        }   
    }
}