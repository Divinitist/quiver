qs_begin
    var N = 7
    var x = 5
    var L = 0
    while (1 << L) < N begin
        L++
    end
    var t = 2 * L + 1
    assign bitnum = t + L
    assign init_val = 1
    circuit "Order Finding" declare

    stage "1st reg all hadamard" start
    upcount (i, 0, t - 1) begin
        apply H at (i)
    end

    stage "apply U" start
    fgate U = func_begin 
        var x_j = 1
        upcount (j, 0, (1 << t) - 1) begin
            upcount (y, 0, (1 << L) - 1) begin
                assign new_state[(j << L) | (x_j * y % N)] += state[(j << L) | y]
            end
            assign x_j = x_j * x % N
        end
    func_end
    call (U)
    
    stage "iqft" start
    var l = 0
    var r = t - 1
    while l < r - 1 begin
        apply SWAP on (l, r)
        assign l += 1
        assign r -= 1
    end
    ugate Ri[t + 1]
    upcount (k, 2, t) begin
        assign Ri[k] = {
            1, 0,
            0, exp(-2.0 * M_PI * im / ((f64)(1 << k)))
        }
    end
    downcount (i, t - 1, 0) begin
        downcount (ci, t - 1, i + 1) begin
            var k = ci - i + 1
            k_apply (Ri, k) by (ci) at (i)
        end
        apply H at (i)
    end
qs_end