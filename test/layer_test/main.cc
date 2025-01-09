#include "layers/layers.h"
#include "../test.h"
#include "InputWarp.h"

#define N 2048          // 输入向量长度
#define M 8192          // midddle
#define batch_size 5
#define vocab_size 12800

void check_embedding();
void check_linear();
void check_rmsnorm();
void check_mlp();
void check_RoPE();

void check_attention() {
    Title("check_attention");

    Tensor<int> input_ids(batch_size, 1);
    input_ids[0] = 0; input_ids[1] = 3324; input_ids[2] = 34; input_ids[3] = 731; input_ids[4] = 7734;

    Tensor<float> x1(batch_size, N);
    rand_init(x1, batch_size*N);
    Tensor<float> x2(x1);

    InputWarp inputWarp(input_ids);
    inputWarp.inter_value = x1;

    const size_t attn_head = 32;
    const size_t kv_head = 8;
    const size_t hidden_size = 2048;
    const size_t max_len = 250;
    const size_t head_dim = hidden_size / hidden_size;
    const size_t q_dim = head_dim*attn_head;
    const size_t kv_dim = head_dim*kv_head;

    std::unordered_map<std::string, std::shared_ptr<float>> weights;
    weights["self_attn.q_linear.weight"] = std::shared_ptr<float>(new float[hidden_size*q_dim]);
    weights["self_attn.k_linear.weight"] = std::shared_ptr<float>(new float[hidden_size*kv_dim]);
    weights["self_attn.v_linear.weight"] = std::shared_ptr<float>(new float[hidden_size*kv_dim]);
    weights["self_attn.o_linear.weight"] = std::shared_ptr<float>(new float[hidden_size*q_dim]);
    rand_init(weights["self_attn.q_linear.weight"].get(), hidden_size*q_dim);
    rand_init(weights["self_attn.k_linear.weight"].get(), hidden_size*kv_dim);
    rand_init(weights["self_attn.v_linear.weight"].get(), hidden_size*kv_dim);
    rand_init(weights["self_attn.o_linear.weight"].get(), hidden_size*q_dim);

    Attention attention(attn_head, kv_head, hidden_size, max_len);
    attention.load(weights);

    attention.forward(inputWarp);
    Tensor<float> y1 = inputWarp.inter_value;
    std::cout << y1[0] << " " << y1[1];
    // inputWarp.inter_value.reset();

    // attention.to("cuda");
    // inputWarp.to("cuda");
    // x2.to("cuda");
    // inputWarp.inter_value = x2;
    // attention.forward(inputWarp);
    // Tensor<float> y2 = inputWarp.inter_value;
    // y2.to("cpu");

    // check(y1, y2, batch_size*N, "ATTENTION");
}


int main() {
    // check_linear();
    // check_rmsnorm();
    
    // check_embedding();
    // check_RoPE();
    check_attention();

    // check_mlp();



}




void check_RoPE() {
    Title("check_RoPE");

    Tensor<int> pos(batch_size, 1);
    for(int i = 0; i < batch_size; i++) pos[i] = i;

    Tensor<float> x1(batch_size, N);
    rand_init(x1, batch_size*N);
    Tensor<float> x2(x1);

    RoPE rope(64);

    rope.forward(x1, pos);

    pos.to("cuda");
    x2.to("cuda");
    rope.to("cuda");
    rope.forward(x2, pos);
    x2.to("cpu");

    check(x1, x2, batch_size*N, "RoPE");
}

void check_embedding() {
    Title("check_embedding");

    std::unordered_map<std::string, std::shared_ptr<float>> weights;
    weights["weight"] = std::shared_ptr<float>(new float[vocab_size*N]);
    rand_init(weights["weight"].get(), N*vocab_size);

    Embedding embedding(vocab_size, N);
    embedding.load(weights);

    Tensor<int> x1(batch_size, 1);
    x1[0] = 0; x1[1] = 3324; x1[2] = 34; x1[3] = 731; x1[4] = 7734;

    Tensor<float> y1 = embedding.forward(x1);

    x1.to("cuda");
    embedding.to("cuda");
    Tensor<float> y2 =embedding.forward(x1);
    y2.to("cpu");

    check(y1, y2, batch_size*N, "EMBEDDING");
}

void check_mlp() {
    Title("check_mlp");

    std::unordered_map<std::string, std::shared_ptr<float>> weights;
    weights["mlp.gate_proj.weight"] = std::shared_ptr<float>(new float[N*M]);
    weights["mlp.up_proj.weight"] = std::shared_ptr<float>(new float[N*M]);
    weights["mlp.down_proj.weight"] = std::shared_ptr<float>(new float[N*M]);

    rand_init(weights["mlp.gate_proj.weight"].get(), N*M);
    rand_init(weights["mlp.up_proj.weight"].get(), N*M);
    rand_init(weights["mlp.down_proj.weight"].get(), N*M);

    Mlp mlp(N, M);
    mlp.load(weights);

    Tensor<float> x1(batch_size, N);
    rand_init(x1, batch_size*N);
    Tensor<float> x2(x1);

    mlp.forward(x1);

    x2.to("cuda");
    mlp.to("cuda");
    mlp.forward(x2);
    x2.to("cpu");

    check(x1, x2, batch_size*N, "MLP");
}

void check_rmsnorm() {
    Title("cheak_rmsnorm");

    std::unordered_map<std::string, std::shared_ptr<float>> weights;
    weights.emplace("weight", std::shared_ptr<float>(new float[N]));

    rand_init(weights["weight"].get(), N);

    RMSNorm rms_norm(N);
    rms_norm.load(weights);

    Tensor<float> x1(batch_size, N);
    rand_init(x1, batch_size*N);
    Tensor<float> x2(x1);

    rms_norm.forward(x1);

    x2.to("cuda");
    rms_norm.to("cuda");
    rms_norm.forward(x2);
    x2.to("cpu");

    check(x1, x2, batch_size*N, "RMS NORM");
}

void check_linear() {
    Title("check_linear");
    size_t size_in = 2;
    size_t size_out = 3;

    std::unordered_map<std::string, std::shared_ptr<float>> weights;
    weights.emplace("weight", std::shared_ptr<float>(new float[size_in*size_out]));

    for(int i = 0; i < size_in * size_out; i++) {
        weights["weight"].get()[i] = i;
    }

    Linear linear(size_in, size_out);
    linear.load(weights);

    size_t num = 2;
    Tensor<float> x(num, size_in);
    x[0] = 1; x[1] = 2; x[2] = 1; x[3] = 3;
    Tensor<float> y = linear.forward(x);

    // 2 8 14 3 11 19
    // for(int i = 0; i < size_out * num; i++)
    //     std::cout << y[i] << " ";
    // std::cout << std::endl;

    x.to("cuda");
    linear.to("cuda");
    Tensor<float> y_cuda = linear.forward(x);
    y_cuda.to("cpu");

    // for(int i = 0; i < size_out * num; i++)
    //     std::cout << y_cuda[i] << " ";
    // std::cout << std::endl;
    check(y_cuda, y, size_out*num, "Linear");
}